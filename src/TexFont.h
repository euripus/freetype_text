#ifndef TEXFONT_H
#define TEXFONT_H

#include <ft2build.h>
#include <string>
#include <vector>
#include FT_FREETYPE_H
#include "AtlasTex.h"

struct Kerning
{
    /**
     * Left character code in the kern pair.
     */
    wchar_t charcode;
    /**
     * Kerning value (in fractional pixels).
     */
    float kerning;

    Kerning(wchar_t charcd, float kern) : charcode(charcd), kerning(kern) {}
};

/**
 * A structure that describe a glyph.
 */
struct TextureGlyph
{
    /**
     * Wide character this glyph represents
     */
    wchar_t charcode;
    /**
     * Glyph id (used for display lists)
     */
    unsigned int id;
    /**
     * Glyph's width in pixels.
     */
    size_t width;
    /**
     * Glyph's height in pixels.
     */
    size_t height;
    /**
     * Glyph's left bearing expressed in integer pixels.
     */
    int offset_x;
    /**
     * Glyphs's top bearing expressed in integer pixels.
     *
     * Remember that this is the distance from the baseline to the top-most
     * glyph scanline, upwards y coordinates being positive.
     */
    int offset_y;
    /**
     * For horizontal text layouts, this is the horizontal distance (in
     * fractional pixels) used to increment the pen position when the glyph is
     * drawn as part of a string of text.
     */
    float advance_x;
    /**
     * For vertical text layouts, this is the vertical distance (in fractional
     * pixels) used to increment the pen position when the glyph is drawn as
     * part of a string of text.
     */
    float advance_y;
    /**
     * First normalized texture coordinate (x) of top-left corner
     */
    float s0;
    /**
     * Second normalized texture coordinate (y) of top-left corner
     */
    float t0;
    /**
     * First normalized texture coordinate (x) of bottom-right corner
     */
    float s1;
    /**
     * Second normalized texture coordinate (y) of bottom-right corner
     */
    float t1;
    /**
     * A vector of kerning pairs relative to this glyph.
     */
    std::vector<Kerning> kerning;
    /**
     * Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
     */
    int outline_type;
    /**
     * Glyph outline thickness
     */
    float outline_thickness;

    TextureGlyph() :
        charcode(-1),
        id(0),
        width(0),
        height(0),
        offset_x(0),
        offset_y(0),
        advance_x(0.0f),
        advance_y(0.0f),
        s0(0.0f),
        t0(0.0f),
        s1(0.0f),
        t1(0.0f),
        outline_type(0),
        outline_thickness(0)
    {}
};

class TexFont
{
public:
    TexFont();
    virtual ~TexFont();

    void Clear();

    bool TextureFontNewFromFile(AtlasTex * atlas, float pt_size, const std::string & filename);
    bool TextureFontNewFromMemory(AtlasTex * atlas, float pt_size, const unsigned char * memory_base,
                                  size_t memory_size);

    TextureGlyph & TextureFontGetGlyph(const wchar_t charcode);
    size_t         TextureFontLoadGlyphs(const wchar_t * charcodes);

    float TextureGlyphGetKerning(const TextureGlyph & self,
                                 const wchar_t charcode) const;   // charcode  codepoint of the peceding glyph
protected:
private:
    bool TexFontLoadFace(float size, FT_Library * library, FT_Face * face);
    bool InitFont();
    void TextureFontGenerateKerning();

    std::vector<TextureGlyph> _glyphs;
    AtlasTex *                _atlas;

    // Font size
    float _size;
    // Whether to use autohint when rendering font
    int _hinting;
    // Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
    int _outline_type;
    // Outline thickness
    float _outline_thickness;
    // Whether to use our own lcd filter.
    int _filtering;
    // Whether to use kerning if available
    int _kerning;
    // LCD filter weights
    unsigned char _lcd_weights[5];
    /**
     * This field is simply used to compute a default line spacing (i.e., the
     * baseline-to-baseline distance) when writing text with this font. Note
     * that it usually is larger than the sum of the ascender and descender
     * taken as absolute values. There is also no guarantee that no glyphs
     * extend above or below subsequent baselines when using this distance.
     */
    float _height;
    /**
     * This field is the distance that must be placed between two lines of
     * text. The baseline-to-baseline distance should be computed as:
     * ascender - descender + linegap
     */
    float _linegap;
    /**
     * The ascender is the vertical distance from the horizontal baseline to
     * the highest 'character' coordinate in a font face. Unfortunately, font
     * formats define the ascender differently. For some, it represents the
     * ascent of all capital latin characters (without accents), for others it
     * is the ascent of the highest accented character, and finally, other
     * formats define it as being equal to bbox.yMax.
     */
    float _ascender;
    /**
     * The descender is the vertical distance from the horizontal baseline to
     * the lowest 'character' coordinate in a font face. Unfortunately, font
     * formats define the descender differently. For some, it represents the
     * descent of all capital latin characters (without accents), for others it
     * is the ascent of the lowest accented character, and finally, other
     * formats define it as being equal to bbox.yMin. This field is negative
     * for values below the baseline.
     */
    float _descender;
    /**
     * The position of the underline line for this face. It is the center of
     * the underlining stem. Only relevant for scalable formats.
     */
    float _underline_position;
    /**
     * The thickness of the underline for this face. Only relevant for scalable
     * formats.
     */
    float _underline_thickness;

    enum
    {
        TEXTURE_FONT_FILE = 0,
        TEXTURE_FONT_MEMORY,
    } _location;
    /**
     * Font filename, for when location == TEXTURE_FONT_FILE
     */
    std::string _filename;
    /**
     * Font memory address, for when location == TEXTURE_FONT_MEMORY
     */
    struct
    {
        const unsigned char * base;
        size_t                size;
    } _memory;
};

#endif   // TEXFONT_H
