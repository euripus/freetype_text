#include "texfont.h"
#include "fontmanager.h"
#include "../VertexBuffer.h"

#include <algorithm>
#include <iostream>

#include <ft2build.h>
#include <cstring>
#include <cwchar>

#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H

// clang-format off
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char * message;
} FT_Errors[] =
#include FT_ERRORS_H
    // clang-format on

    constexpr std::uint32_t HRES  = 64;
constexpr float             HRESf = 64.0f;
constexpr std::uint32_t     DPI   = 72;

static bool TexFontLoadFace(float size, FT_Library * library, FT_Face * face, TexFont::FontLocation location,
                            std::string const & filename, std::vector<unsigned char> const & data)
{
    assert(library);
    assert(size);

    FT_Error  error;
    FT_Matrix matrix = {static_cast<int>((1.0 / HRES) * 0x10000L), static_cast<int>((0.0) * 0x10000L),
                        static_cast<int>((0.0) * 0x10000L), static_cast<int>((1.0) * 0x10000L)};

    /* Initialize library */
    error = FT_Init_FreeType(library);

    if(error)
    {
        std::cerr << "FT_Error " << FT_Errors[error].code << ": " << FT_Errors[error].message << std::endl;
        return false;
    }

    /* Load face */
    switch(location)
    {
        case TexFont::FontLocation::TEXTURE_FONT_FILE:
            error = FT_New_Face(*library, filename.c_str(), 0, face);
            break;

        case TexFont::FontLocation::TEXTURE_FONT_MEMORY:
            error = FT_New_Memory_Face(*library, data.data(), static_cast<FT_Long>(data.size()), 0, face);
            break;
    }

    if(error)
    {
        std::cerr << "FT_Error line " << __LINE__ << ", code " << FT_Errors[error].code << ": "
                  << FT_Errors[error].message << std::endl;
        FT_Done_FreeType(*library);
        return false;
    }

    /* Select charmap */
    error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);

    if(error)
    {
        std::cerr << "FT_Error line " << __LINE__ << ", code " << FT_Errors[error].code << ": "
                  << FT_Errors[error].message << std::endl;
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        return false;
    }

    /* Set char size */
    error = FT_Set_Char_Size(*face, static_cast<int>(size * HRES), 0, DPI * HRES, DPI);

    if(error)
    {
        std::cerr << "FT_Error line " << __LINE__ << ", code " << FT_Errors[error].code << ": "
                  << FT_Errors[error].message << std::endl;
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        return false;
    }

    /* Set transform matrix */
    FT_Set_Transform(*face, &matrix, NULL);
    return true;
}

TexFont::TexFont(FontManager & owner, std::string const & filename, float pt_size, bool hinting, bool kerning,
                 float outline_thickness, Glyph::OutlineType outline_type) :
    m_owner{owner},
    m_size{pt_size},
    m_hinting{hinting},
    m_outline_type{outline_type},
    m_outline_thickness{outline_thickness},
    m_kerning{kerning},
    m_lcd_weights{},
    m_height{0.0f},
    m_linegap{0.0f},
    m_ascender{0.0f},
    m_descender{0.0f},
    m_underline_position{0.0f},
    m_underline_thickness{0.0f},
    m_location{FontLocation::TEXTURE_FONT_FILE},
    m_filename{filename}
{
    assert(!filename.empty());
    assert(pt_size > 0);

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    m_lcd_weights[0] = 0x10;
    m_lcd_weights[1] = 0x40;
    m_lcd_weights[2] = 0x70;
    m_lcd_weights[3] = 0x40;
    m_lcd_weights[4] = 0x10;

    if(!initFont())
        throw std::runtime_error("Error while loading font from file!!!");
}

TexFont::TexFont(FontManager & owner, unsigned char const * memory_base, size_t memory_size, float pt_size,
                 bool hinting, bool kerning, float outline_thickness, Glyph::OutlineType outline_type) :
    m_owner{owner},
    m_size{pt_size},
    m_hinting{hinting},
    m_outline_type{outline_type},
    m_outline_thickness{outline_thickness},
    m_kerning{kerning},
    m_lcd_weights{},
    m_height{0.0f},
    m_linegap{0.0f},
    m_ascender{0.0f},
    m_descender{0.0f},
    m_underline_position{0.0f},
    m_underline_thickness{0.0f},
    m_location{FontLocation::TEXTURE_FONT_MEMORY}
{
    assert(pt_size > 0);
    assert(memory_base);
    assert(memory_size > 0);

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    m_lcd_weights[0] = 0x10;
    m_lcd_weights[1] = 0x40;
    m_lcd_weights[2] = 0x70;
    m_lcd_weights[3] = 0x40;
    m_lcd_weights[4] = 0x10;

    m_memory.resize(memory_size);
    std::memcpy(m_memory.data(), memory_base, memory_size);

    if(!initFont())
        throw std::runtime_error("Error while loading font from memory!!!");
}

bool TexFont::initFont()
{
    FT_Library      library;
    FT_Face         face;
    FT_Size_Metrics metrics;

    if(!TexFontLoadFace(m_size, &library, &face, m_location, m_filename, m_memory))
    {
        return false;
    }

    m_underline_position = face->underline_position / (HRESf * HRESf) * m_size;
    m_underline_position = roundf(m_underline_position);
    if(m_underline_position > -2.0f)
    {
        m_underline_position = -2.0f;
    }

    m_underline_thickness = face->underline_thickness / (HRESf * HRESf) * m_size;
    m_underline_thickness = roundf(m_underline_thickness);
    if(m_underline_thickness < 1.0f)
    {
        m_underline_thickness = 1.0f;
    }

    metrics     = face->size->metrics;
    m_ascender  = (metrics.ascender >> 6) / 100.0f;
    m_descender = (metrics.descender >> 6) / 100.0f;
    m_height    = (metrics.height >> 6) / 100.0f;
    m_linegap   = m_height - m_ascender + m_descender;
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    /* -1 is a special glyph */
    loadGlyph(-1);

    return true;
}

Glyph const & TexFont::getGlyph(const std::uint32_t ucodepoint) const
{
    // Check if charcode has been already loaded
    for(std::uint32_t i = 0; i < m_glyphs.size(); ++i)
    {
        Glyph const & glyph = m_glyphs[i];
        // If charcode is -1, we don't care about outline type or thickness
        if((glyph.charcode == ucodepoint)
           && ((ucodepoint == static_cast<std::uint32_t>(-1))
               || ((glyph.outline_type == m_outline_type)
                   && (glyph.outline_thickness == m_outline_thickness))))
        {
            return glyph;
        }
    }

    return m_glyphs[0];
}

std::int32_t TexFont::loadGlyph(char const * charcode)
{
    std::uint32_t ucodepoint = 0;
    // codepoint NULL is special : it is used for line drawing (overline, underline, strikethrough) and
    // background.
    if(charcode == nullptr)
        ucodepoint = static_cast<std::uint32_t>(-1);
    else
        ucodepoint = utf8_to_utf32(charcode);

    return loadGlyph(ucodepoint);
}

std::int32_t TexFont::loadGlyph(std::uint32_t ucodepoint)
{
    size_t       x, y, w, h, size;
    FT_Library   library;
    FT_Error     error;
    FT_Face      face;
    FT_Glyph     ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap    ft_bitmap;

    FT_UInt    glyph_index;
    glm::ivec4 region;

    size = m_owner.getAtlas().getSize();

    // Check if charcode has been already loaded
    for(std::uint32_t i = 0; i < m_glyphs.size(); ++i)
    {
        Glyph const & glyph = m_glyphs[i];
        // If charcode is -1, we don't care about outline type or thickness
        if((glyph.charcode == ucodepoint)
           && ((ucodepoint == static_cast<std::uint32_t>(-1))
               || ((glyph.outline_type == m_outline_type)
                   && (glyph.outline_thickness == m_outline_thickness))))
        {
            return i;
        }
    }

    if(ucodepoint == static_cast<std::uint32_t>(-1))
    {
        Glyph      glyph;
        size_t     size   = m_owner.getAtlas().getSize();
        glm::ivec4 region = m_owner.getAtlas().getRegion(5, 5);

        static unsigned char data[4 * 4 * 3] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
        if(region.x < 0)
        {
            std::cerr << "Texture atlas is full " << __LINE__ << std::endl;
            return -1;
        }

        m_owner.getAtlas().setRegionTL(glm::ivec4(region.x, region.y, 4, 4), data, 0);
        glyph.charcode = static_cast<std::uint32_t>(-1);
        glyph.s0       = (region.x + 2) / static_cast<float>(size);
        glyph.t0       = (region.y + 2) / static_cast<float>(size);
        glyph.s1       = (region.x + 3) / static_cast<float>(size);
        glyph.t1       = (region.y + 3) / static_cast<float>(size);
        m_glyphs.push_back(std::move(glyph));
        return m_glyphs.size() - 1;
    }

    if(!TexFontLoadFace(m_size, &library, &face, m_location, m_filename, m_memory))
        return 0;

    FT_Int32 flags         = 0;
    int      ft_glyph_top  = 0;
    int      ft_glyph_left = 0;
    glyph_index            = FT_Get_Char_Index(face, ucodepoint);

    if(m_outline_type != Glyph::OutlineType::NONE)
    {
        flags |= FT_LOAD_NO_BITMAP;
    }
    else
    {
        flags |= FT_LOAD_RENDER;
    }

    if(!m_hinting)
    {
        flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
    }
    else
    {
        flags |= FT_LOAD_FORCE_AUTOHINT;
    }

    FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
    flags |= FT_LOAD_TARGET_LCD;
    FT_Library_SetLcdFilterWeights(library, m_lcd_weights);

    error = FT_Load_Glyph(face, glyph_index, flags);
    if(error)
    {
        std::cerr << "FT_Error line " << __LINE__ << ", code " << FT_Errors[error].code << ": "
                  << FT_Errors[error].message << std::endl;
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 0;
    }

    if(m_outline_type == Glyph::OutlineType::NONE)
    {
        slot          = face->glyph;
        ft_bitmap     = slot->bitmap;
        ft_glyph_top  = slot->bitmap_top;
        ft_glyph_left = slot->bitmap_left;
    }
    else
    {
        FT_Stroker     stroker;
        FT_BitmapGlyph ft_bitmap_glyph;
        error = FT_Stroker_New(library, &stroker);
        if(error)
        {
            std::cerr << "FT_Error code " << FT_Errors[error].code << ": " << FT_Errors[error].message
                      << std::endl;
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }
        FT_Stroker_Set(stroker, static_cast<int>(m_outline_thickness * HRES), FT_STROKER_LINECAP_ROUND,
                       FT_STROKER_LINEJOIN_ROUND, 0);
        error = FT_Get_Glyph(face->glyph, &ft_glyph);
        if(error)
        {
            std::cerr << "FT_Error code " << FT_Errors[error].code << ": " << FT_Errors[error].message
                      << std::endl;
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }

        if(m_outline_type == Glyph::OutlineType::LINE)
        {
            error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
        }
        else if(m_outline_type == Glyph::OutlineType::INNER)
        {
            error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
        }
        else if(m_outline_type == Glyph::OutlineType::OUTER)
        {
            error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
        }
        if(error)
        {
            std::cerr << "FT_Error code " << FT_Errors[error].code << ": " << FT_Errors[error].message
                      << std::endl;
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }

        error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
        if(error)
        {
            std::cerr << "FT_Error code " << FT_Errors[error].code << ": " << FT_Errors[error].message
                      << std::endl;
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }

        ft_bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(ft_glyph);
        ft_bitmap       = ft_bitmap_glyph->bitmap;
        ft_glyph_top    = ft_bitmap_glyph->top;
        ft_glyph_left   = ft_bitmap_glyph->left;
        FT_Stroker_Done(stroker);
    }

    // We want each glyph to be separated by at least one black pixel
    w      = ft_bitmap.width / 3 + 1;
    h      = ft_bitmap.rows + 1;
    region = m_owner.getAtlas().getRegion(w, h);
    if(region.x < 0)
    {
        std::cerr << "Texture atlas is full " << __LINE__ << std::endl;
        return -1;
    }

    w = w - 1;
    h = h - 1;
    x = region.x;
    y = region.y;
    m_owner.getAtlas().setRegionTL(glm::ivec4(x, y, w, h), ft_bitmap.buffer, ft_bitmap.pitch);

    Glyph glyph;
    glyph.charcode          = ucodepoint;
    glyph.width             = w;
    glyph.height            = h;
    glyph.outline_type      = m_outline_type;
    glyph.outline_thickness = m_outline_thickness;
    glyph.offset_x          = ft_glyph_left;
    glyph.offset_y          = ft_glyph_top;
    glyph.s0                = x / static_cast<float>(size);
    glyph.t0                = (y + 1) / static_cast<float>(size);   // one black pixel margin
    glyph.s1                = (x + glyph.width) / static_cast<float>(size);
    glyph.t1                = (y + 1 + glyph.height) / static_cast<float>(size);

    // Discard hinting to get advance
    FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
    slot            = face->glyph;
    glyph.advance_x = slot->advance.x / HRESf;
    glyph.advance_y = slot->advance.y / HRESf;

    m_glyphs.push_back(std::move(glyph));

    if(m_outline_type != Glyph::OutlineType::NONE)
    {
        FT_Done_Glyph(ft_glyph);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return m_glyphs.size() - 1;
}

size_t TexFont::cacheGlyphs(char const * charcodes)
{
    assert(charcodes);

    std::uint32_t missed = 0;

    // Load each glyph
    for(std::uint32_t i = 0; i < std::strlen(charcodes); i += utf8_surrogate_len(charcodes + i))
    {
        auto error = loadGlyph(charcodes + i);

        if(error == 0)   // error loading glyph
            missed++;

        if(error == -1)   // atlas full
        {
            m_owner.resizeAtlas();

            // repeat load glyph
            if(!loadGlyph(charcodes + i))
                missed++;
        }
    }

    if(m_kerning)
        generateKerning();

    return missed;
}

void TexFont::generateKerning()
{
    for(auto & glyph : m_glyphs)
    {
        generateKerning(glyph);
    }
}

void TexFont::generateKerning(Glyph & glyph)
{
    FT_Library library;
    FT_Face    face;
    FT_UInt    glyph_index, prev_index;
    FT_Vector  kerning;

    /* Load font */
    if(!TexFontLoadFace(m_size, &library, &face, m_location, m_filename, m_memory))
        return;

    glyph_index = FT_Get_Char_Index(face, glyph.charcode);
    glyph.kerning.clear();

    for(auto & prev_glyph : m_glyphs)
    {
        prev_index = FT_Get_Char_Index(face, prev_glyph.charcode);
        FT_Get_Kerning(face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);

        if(kerning.x)
        {
            glyph.kerning[prev_glyph.charcode] = kerning.x / (HRESf * HRESf);
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

float TexFont::glyphGetKerning(Glyph const & glyph, const std::uint32_t left_charcode) const
{
    if(glyph.kerning.empty())
        return 0.0f;

    if(auto search = glyph.kerning.find(left_charcode); search != glyph.kerning.end())
        return search->second;
    else
        return 0.0f;
}

glm::vec2 TexFont::getTextSize(char const * text)
{
    glm::vec2     size{0};
    Glyph const * prev_glyph = nullptr;

    for(unsigned int i = 0; i < std::strlen(text); i += utf8_surrogate_len(text + i))
    {
        std::uint32_t ucodepoint = utf8_to_utf32(text + i);
        Glyph const & glyph      = getGlyph(ucodepoint);

        float kerning = 0.0f;
        if(prev_glyph != nullptr && m_kerning)
        {
            kerning = glyphGetKerning(glyph, prev_glyph->charcode);
        }
        prev_glyph = &glyph;
        size.x += kerning;

        size.y = glm::max(size.y, static_cast<float>(glyph.offset_y));
        size.x += glyph.advance_x;
    }

    return size;
}

void TexFont::addText(VertexBuffer & vb, char const * text, glm::vec2 & pos)
{
    Glyph const * prev_glyph = nullptr;
    for(unsigned int i = 0; i < std::strlen(text); i += utf8_surrogate_len(text + i))
    {
        std::uint32_t ucodepoint = utf8_to_utf32(text + i);
        Glyph const & glyph      = getGlyph(ucodepoint);

        float kerning = 0.0f;
        if(prev_glyph != nullptr && m_kerning)
        {
            kerning = glyphGetKerning(glyph, prev_glyph->charcode);
        }
        prev_glyph = &glyph;

        pos.x += kerning;
        float        x0         = pos.x + glyph.offset_x;
        float        y0         = pos.y + (glyph.offset_y - static_cast<int>(glyph.height));
        float        x1         = x0 + static_cast<int>(glyph.width);
        float        y1         = pos.y + glyph.offset_y;
        float        s0         = glyph.s0;
        float        t0         = glyph.t0;
        float        s1         = glyph.s1;
        float        t1         = glyph.t1;
        unsigned int indices[6] = {0, 1, 2, 0, 3, 1};

        float vertices[4 * 5] = {x0, y0, 0.0, s0, t0, x1, y1, 0.0, s1, t1,
                                 x0, y1, 0.0, s0, t1, x1, y0, 0.0, s1, t0};

        vb.VertexBufferPushBack(vertices, 4, indices, 6);
        pos.x += glyph.advance_x;
    }
}

void TexFont::reloadGlyphs()
{
    std::vector<std::uint32_t> loaded_ucodepoints;

    // copy Unicode codepoints from already loaded glyphs
    std::for_each(begin(m_glyphs), end(m_glyphs), [&loaded_ucodepoints](Glyph const & glyph) {
        loaded_ucodepoints.push_back(glyph.charcode);
    });

    // clear glyphs
    m_glyphs.resize(0);

    // load glyphs to new atlas
    for(auto const & ucodepoint : loaded_ucodepoints)
    {
        loadGlyph(ucodepoint);
    }
}
