#include "TexFont.h"
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
constexpr float         HRESf = 64.0f;
constexpr std::uint32_t DPI   = 72;

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
            error = FT_New_Memory_Face(*library, data.data(), data.size(), 0, face);
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
    error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

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

TexFont::TexFont(float pt_size, std::string const & filename) :
    _size{pt_size},
    _hinting{1},
    _outline_type{0},
    _outline_thickness{0.0f},
    _filtering{1},
    _kerning{1},
    _lcd_weights{},
    _height{0.0f},
    _linegap{0.0f},
    _ascender{0.0f},
    _descender{0.0f},
    _underline_position{0.0f},
    _underline_thickness{0.0f},
    _location{FontLocation::TEXTURE_FONT_FILE},
    _filename{filename}
{
    assert(!filename.empty());
    assert(pt_size > 0);

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    _lcd_weights[0] = 0x10;
    _lcd_weights[1] = 0x40;
    _lcd_weights[2] = 0x70;
    _lcd_weights[3] = 0x40;
    _lcd_weights[4] = 0x10;

    if(!initFont())
        throw std::runtime_error("Error while loading font from file!!!");
}

TexFont::TexFont(float pt_size, unsigned char const * memory_base, size_t memory_size) :
    _size{pt_size},
    _hinting{1},
    _outline_type{0},
    _outline_thickness{0.0f},
    _filtering{1},
    _kerning{1},
    _lcd_weights{},
    _height{0.0f},
    _linegap{0.0f},
    _ascender{0.0f},
    _descender{0.0f},
    _underline_position{0.0f},
    _underline_thickness{0.0f},
    _location{FontLocation::TEXTURE_FONT_MEMORY}
{
    assert(pt_size > 0);
    assert(memory_base);
    assert(memory_size > 0);

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    _lcd_weights[0] = 0x10;
    _lcd_weights[1] = 0x40;
    _lcd_weights[2] = 0x70;
    _lcd_weights[3] = 0x40;
    _lcd_weights[4] = 0x10;

    _memory.resize(memory_size);
    std::memcpy(_memory.data(), memory_base, memory_size);

    if(!initFont())
        throw std::runtime_error("Error while loading font from memory!!!");
}

bool TexFont::initFont()
{
    FT_Library      library;
    FT_Face         face;
    FT_Size_Metrics metrics;

    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
    {
        return false;
    }

    _underline_position = face->underline_position / (HRESf * HRESf) * _size;
    _underline_position = round(_underline_position);
    if(_underline_position > -2.0f)
    {
        _underline_position = -2.0f;
    }

    _underline_thickness = face->underline_thickness / (HRESf * HRESf) * _size;
    _underline_thickness = round(_underline_thickness);
    if(_underline_thickness < 1.0f)
    {
        _underline_thickness = 1.0f;
    }

    metrics    = face->size->metrics;
    _ascender  = (metrics.ascender >> 6) / 100.0f;
    _descender = (metrics.descender >> 6) / 100.0f;
    _height    = (metrics.height >> 6) / 100.0f;
    _linegap   = _height - _ascender + _descender;
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    /* -1 is a special glyph */
    textureFontLoadGlyph(-1);

    return true;
}

Glyph & TexFont::textureFontGetGlyph(const std::uint32_t ucodepoint)
{
    // Check if charcode has been already loaded
    for(std::uint32_t i = 0; i < _glyphs.size(); ++i)
    {
        Glyph & glyph = _glyphs[i];
        // If charcode is -1, we don't care about outline type or thickness
        if((glyph.charcode == ucodepoint)
           && ((ucodepoint == static_cast<std::uint32_t>(-1))
               || ((glyph.outline_type == _outline_type) && (glyph.outline_thickness == _outline_thickness))))
        {
            return glyph;
        }
    }

    // Glyph has not been already loaded
    auto i = textureFontLoadGlyph(ucodepoint);
    if(i > 0)
    {
        textureFontGenerateKerning(_glyphs[i]);
        return _glyphs[i];
    }
    else if(i == -1)   // atlas full
    {
        resizeAtlas();
        i = textureFontLoadGlyph(ucodepoint);
        if(i > 0)
        {
            textureFontGenerateKerning();
            return _glyphs[i];
        }
    }

    throw std::runtime_error("Glyph not found!!!");
}

std::int32_t TexFont::textureFontLoadGlyph(char const * charcode)
{
    std::uint32_t ucodepoint = 0;
    // codepoint NULL is special : it is used for line drawing (overline, underline, strikethrough) and
    // background.
    if(charcode == nullptr)
        ucodepoint = static_cast<std::uint32_t>(-1);
    else
        ucodepoint = utf8_to_utf32(charcode);

    return textureFontLoadGlyph(ucodepoint);
}

std::int32_t TexFont::textureFontLoadGlyph(std::uint32_t ucodepoint)
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

    size = _atlas.getSize();

    // Check if charcode has been already loaded
    for(std::uint32_t i = 0; i < _glyphs.size(); ++i)
    {
        Glyph & glyph = _glyphs[i];
        // If charcode is -1, we don't care about outline type or thickness
        if((glyph.charcode == ucodepoint)
           && ((ucodepoint == static_cast<std::uint32_t>(-1))
               || ((glyph.outline_type == _outline_type) && (glyph.outline_thickness == _outline_thickness))))
        {
            return i;
        }
    }

    if(ucodepoint == static_cast<std::uint32_t>(-1))
    {
        Glyph      glyph;
        size_t     size   = _atlas.getSize();
        glm::ivec4 region = _atlas.getRegion(5, 5);

        static unsigned char data[4 * 4 * 3] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
        if(region.x < 0)
        {
            std::cerr << "Texture atlas is full " << __LINE__ << std::endl;
            return -1;
        }

        _atlas.setRegion(glm::ivec4(region.x, region.y, 4, 4), data, 0);
        glyph.charcode = static_cast<std::uint32_t>(-1);
        glyph.s0       = (region.x + 2) / static_cast<float>(size);
        glyph.t0       = (region.y + 2) / static_cast<float>(size);
        glyph.s1       = (region.x + 3) / static_cast<float>(size);
        glyph.t1       = (region.y + 3) / static_cast<float>(size);
        _glyphs.push_back(std::move(glyph));
        return _glyphs.size() - 1;
    }

    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
        return 0;

    FT_Int32 flags         = 0;
    int      ft_glyph_top  = 0;
    int      ft_glyph_left = 0;
    glyph_index            = FT_Get_Char_Index(face, ucodepoint);

    if(_outline_type > 0)
    {
        flags |= FT_LOAD_NO_BITMAP;
    }
    else
    {
        flags |= FT_LOAD_RENDER;
    }

    if(!_hinting)
    {
        flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
    }
    else
    {
        flags |= FT_LOAD_FORCE_AUTOHINT;
    }

    FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
    flags |= FT_LOAD_TARGET_LCD;
    if(_filtering)
    {
        FT_Library_SetLcdFilterWeights(library, _lcd_weights);
    }

    error = FT_Load_Glyph(face, glyph_index, flags);
    if(error)
    {
        std::cerr << "FT_Error line " << __LINE__ << ", code " << FT_Errors[error].code << ": "
                  << FT_Errors[error].message << std::endl;
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 0;
    }

    if(_outline_type == 0)
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
        FT_Stroker_Set(stroker, (int)(_outline_thickness * HRES), FT_STROKER_LINECAP_ROUND,
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

        if(_outline_type == 1)
        {
            error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
        }
        else if(_outline_type == 2)
        {
            error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
        }
        else if(_outline_type == 3)
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

        ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
        ft_bitmap       = ft_bitmap_glyph->bitmap;
        ft_glyph_top    = ft_bitmap_glyph->top;
        ft_glyph_left   = ft_bitmap_glyph->left;
        FT_Stroker_Done(stroker);
    }

    // We want each glyph to be separated by at least one black pixel
    // (for example for shader used in demo-subpixel.c)
    w      = ft_bitmap.width / 3 + 1;
    h      = ft_bitmap.rows + 1;
    region = _atlas.getRegion(w, h);
    if(region.x < 0)
    {
        std::cerr << "Texture atlas is full " << __LINE__ << std::endl;
        return -1;
    }

    w = w - 1;
    h = h - 1;
    x = region.x;
    y = region.y;
    _atlas.setRegion(glm::ivec4(x, y, w, h), ft_bitmap.buffer, ft_bitmap.pitch);

    Glyph glyph;
    glyph.charcode          = ucodepoint;
    glyph.width             = w;
    glyph.height            = h;
    glyph.outline_type      = _outline_type;
    glyph.outline_thickness = _outline_thickness;
    glyph.offset_x          = ft_glyph_left;
    glyph.offset_y          = ft_glyph_top;
    glyph.s0                = x / static_cast<float>(size);
    glyph.t0                = y / static_cast<float>(size);
    glyph.s1                = (x + glyph.width) / static_cast<float>(size);
    glyph.t1                = (y + glyph.height) / static_cast<float>(size);

    // Discard hinting to get advance
    FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
    slot            = face->glyph;
    glyph.advance_x = slot->advance.x / HRESf;
    glyph.advance_y = slot->advance.y / HRESf;

    _glyphs.push_back(std::move(glyph));

    if(_outline_type > 0)
    {
        FT_Done_Glyph(ft_glyph);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return _glyphs.size() - 1;
}

size_t TexFont::textureFontCacheGlyphs(char const * charcodes)
{
    assert(charcodes);

    std::uint32_t missed = 0;

    // Load each glyph
    for(std::uint32_t i = 0; i < std::strlen(charcodes); i += utf8_surrogate_len(charcodes + i))
    {
        auto error = textureFontLoadGlyph(charcodes + i);

        if(error == 0)   // error loading glyph
            missed++;

        if(error == -1)   // atlas full
        {
            resizeAtlas();

            // repeat load glyph
            if(!textureFontLoadGlyph(charcodes + i))
                missed++;
        }
    }

    textureFontGenerateKerning();

    return missed;
}

void TexFont::textureFontGenerateKerning()
{
    FT_Library library;
    FT_Face    face;
    FT_UInt    glyph_index, prev_index;
    FT_Vector  kerning;

    /* Load font */
    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
        return;

    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for(auto & glyph : _glyphs)
    {
        glyph_index = FT_Get_Char_Index(face, glyph.charcode);
        glyph.kerning.clear();

        for(auto & prev_glyph : _glyphs)
        {
            prev_index = FT_Get_Char_Index(face, prev_glyph.charcode);
            FT_Get_Kerning(face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);

            if(kerning.x)
            {
                glyph.kerning[prev_glyph.charcode] = kerning.x / (HRESf * HRESf);
            }
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void TexFont::textureFontGenerateKerning(Glyph & glyph)
{
    FT_Library library;
    FT_Face    face;
    FT_UInt    glyph_index, prev_index;
    FT_Vector  kerning;

    /* Load font */
    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
        return;

    glyph_index = FT_Get_Char_Index(face, glyph.charcode);
    glyph.kerning.clear();

    for(auto & prev_glyph : _glyphs)
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

void TexFont::resizeAtlas()
{
    AtlasTex new_atlas(_atlas.getSize() * 2);
    _atlas = new_atlas;

    std::vector<std::uint32_t> loaded_ucodepoints;

    // copy Unicode codepoints from already loaded glyphs
    std::for_each(begin(_glyphs), end(_glyphs), [&loaded_ucodepoints](Glyph const & glyph) {
        loaded_ucodepoints.push_back(glyph.charcode);
    });

    // clear glyphs
    _glyphs.resize(0);

    // load glyphs to new atlas
    for(auto const & ucodepoint : loaded_ucodepoints)
    {
        textureFontLoadGlyph(ucodepoint);
    }
}
