#include "TexFont.h"
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
    FT_Matrix matrix = {(int)((1.0 / HRES) * 0x10000L), (int)((0.0) * 0x10000L), (int)((0.0) * 0x10000L),
                        (int)((1.0) * 0x10000L)};

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

bool TexFont::InitFont()
{
    FT_Library      library;
    FT_Face         face;
    FT_Size_Metrics metrics;

    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
    {
        return false;
    }

    _underline_position = face->underline_position / (float)(HRESf * HRESf) * _size;
    _underline_position = round(_underline_position);
    if(_underline_position > -2)
    {
        _underline_position = -2.0;
    }

    _underline_thickness = face->underline_thickness / (float)(HRESf * HRESf) * _size;
    _underline_thickness = round(_underline_thickness);
    if(_underline_thickness < 1)
    {
        _underline_thickness = 1.0;
    }

    metrics    = face->size->metrics;
    _ascender  = (metrics.ascender >> 6) / 100.0;
    _descender = (metrics.descender >> 6) / 100.0;
    _height    = (metrics.height >> 6) / 100.0;
    _linegap   = _height - _ascender + _descender;
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    /* -1 is a special glyph */
    TextureFontGetGlyph(-1);

    return true;
}

TexFont::TexFont()
{
    _height            = 0;
    _ascender          = 0;
    _descender         = 0;
    _outline_type      = 0;
    _outline_thickness = 0.0;
    _hinting           = 1;
    _kerning           = 1;
    _filtering         = 1;

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    _lcd_weights[0] = 0x10;
    _lcd_weights[1] = 0x40;
    _lcd_weights[2] = 0x70;
    _lcd_weights[3] = 0x40;
    _lcd_weights[4] = 0x10;

    _atlas = nullptr;
}

bool TexFont::TextureFontNewFromFile(AtlasTex * atlas, float pt_size, std::string const & filename)
{
    assert(!filename.empty());
    assert(atlas);
    assert(pt_size > 0);

    _atlas = atlas;
    _size  = pt_size;

    _location = TexFont::FontLocation::TEXTURE_FONT_FILE;
    _filename = filename;

    if(!InitFont())
        return false;
    return true;
}

bool TexFont::TextureFontNewFromMemory(AtlasTex * atlas, float pt_size, unsigned char const * memory_base,
                                       size_t memory_size)
{
    assert(atlas);
    assert(pt_size > 0);
    assert(memory_base);
    assert(memory_size > 0);

    _atlas = atlas;
    _size  = pt_size;

    _location = TexFont::FontLocation::TEXTURE_FONT_MEMORY;
    _memory.resize(memory_size);
    std::memcpy(_memory.data(), memory_base, memory_size);

    if(!InitFont())
        return false;
    return true;
}

Glyph & TexFont::TextureFontGetGlyph(const wchar_t charcode)
{
    size_t  i;
    wchar_t buffer[2] = {0, 0};

    assert(_atlas);

    /* Check if charcode has been already loaded */
    for(i = 0; i < _glyphs.size(); ++i)
    {
        Glyph & glyph = _glyphs[i];
        // If charcode is -1, we don't care about outline type or thickness
        if((glyph.charcode == charcode)
           && ((charcode == (wchar_t)(-1))
               || ((glyph.outline_type == _outline_type) && (glyph.outline_thickness == _outline_thickness))))
        {
            return glyph;
        }
    }

    /* charcode -1 is special : it is used for line drawing (overline,
     * underline, strikethrough) and background.
     */
    if(charcode == (wchar_t)(-1))
    {
        Glyph      glyph;
        size_t     size   = _atlas->getSize();
        glm::ivec4 region = _atlas->getRegion(5, 5);

        static unsigned char data[4 * 4 * 3] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
        if(region.x < 0)
        {
            std::cerr << "Texture atlas is full " << __LINE__ << std::endl;
            return glyph;
        }

        _atlas->setRegion(glm::ivec4(region.x, region.y, 4, 4), data, 0);
        glyph.charcode = (wchar_t)(-1);
        glyph.s0       = (region.x + 2) / (float)size;
        glyph.t0       = (region.y + 2) / (float)size;
        glyph.s1       = (region.x + 3) / (float)size;
        glyph.t1       = (region.y + 3) / (float)size;
        _glyphs.push_back(std::move(glyph));
        return _glyphs.back();
    }

    /* Glyph has not been already loaded */
    buffer[0] = charcode;
    if(TextureFontLoadGlyphs(buffer) == 0)
    {
        return _glyphs.back();
    }
}

size_t TexFont::TextureFontLoadGlyphs(wchar_t const * charcodes)
{
    assert(charcodes);

    size_t       i, j, x, y, w, h, size;
    FT_Library   library;
    FT_Error     error;
    FT_Face      face;
    FT_Glyph     ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap    ft_bitmap;

    FT_UInt    glyph_index;
    glm::ivec4 region;
    size_t     missed = 0;

    char pass;
    size = _atlas->getSize();

    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
        return std::wcslen(charcodes);

    for(i = 0; i < std::wcslen(charcodes); ++i)
    {
        pass = 0;
        /* Check if charcode has been already loaded */
        for(j = 0; j < _glyphs.size(); ++j)
        {
            Glyph & glyph = _glyphs[j];
            // If charcode is -1, we don't care about outline type or thickness
            if((glyph.charcode == charcodes[i]))
            {
                pass = 1;
                break;
            }
        }

        if(pass)
            continue;   // don't add the item

        FT_Int32 flags         = 0;
        int      ft_glyph_top  = 0;
        int      ft_glyph_left = 0;
        glyph_index            = FT_Get_Char_Index(face, charcodes[i]);
        // WARNING: We use texture-atlas depth to guess if user wants
        //          LCD subpixel rendering

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
            return wcslen(charcodes) - i;
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
        region = _atlas->getRegion(w, h);
        if(region.x < 0)
        {
            missed++;
            std::cerr << "Texture atlas is full " << __LINE__ << std::endl;
            continue;
        }
        w = w - 1;
        h = h - 1;
        x = region.x;
        y = region.y;
        _atlas->setRegion(glm::ivec4(x, y, w, h), ft_bitmap.buffer, ft_bitmap.pitch);

        Glyph glyph;
        glyph.charcode          = charcodes[i];
        glyph.width             = w;
        glyph.height            = h;
        glyph.outline_type      = _outline_type;
        glyph.outline_thickness = _outline_thickness;
        glyph.offset_x          = ft_glyph_left;
        glyph.offset_y          = ft_glyph_top;
        glyph.s0                = x / (float)size;
        glyph.t0                = y / (float)size;
        glyph.s1                = (x + glyph.width) / (float)size;
        glyph.t1                = (y + glyph.height) / (float)size;

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
    }
    FT_Done_Face(face);
    FT_Done_FreeType(library);
    TextureFontGenerateKerning();

    return missed;
}

void TexFont::TextureFontGenerateKerning()
{
    size_t     i, j;
    FT_Library library;
    FT_Face    face;
    FT_UInt    glyph_index, prev_index;
    FT_Vector  kerning;

    /* Load font */
    if(!TexFontLoadFace(_size, &library, &face, _location, _filename, _memory))
        return;

    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for(i = 1; i < _glyphs.size(); ++i)
    {
        Glyph & glyph = _glyphs[i];
        glyph_index   = FT_Get_Char_Index(face, glyph.charcode);
        glyph.kerning.clear();

        for(j = 1; j < _glyphs.size(); ++j)
        {
            Glyph & prev_glyph = _glyphs[j];
            prev_index         = FT_Get_Char_Index(face, prev_glyph.charcode);
            FT_Get_Kerning(face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);

            if(kerning.x)
            {
                Kerning k{prev_glyph.charcode, kerning.x / (float)(HRESf * HRESf)};
                glyph.kerning.push_back(k);
            }
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

float TexFont::GlyphGetKerning(Glyph const & glyph, const std::uint32_t charcode) const
{
    for(auto const & kerning : glyph.kerning)
    {
        if(kerning.left_charcode == charcode)
        {
            return kerning.kerning;
        }
    }
    return 0;
}
