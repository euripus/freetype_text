#ifndef TEXFONT_H
#define TEXFONT_H

#include <map>
#include <vector>
#include "AtlasTex.h"
#include "utf8_utils.h"

struct Glyph
{
    std::uint32_t charcode     = -1;       // Wide character this glyph represents
    size_t        width        = 0;        // Glyph's width in pixels
    size_t        height       = 0;        // Glyph's height in pixels.
    int           offset_x     = 0;        // Glyph's left bearing expressed in integer pixels.
    int           offset_y     = 0;        // Glyphs's top bearing expressed in integer pixels.
    float         advance_x    = 0.0f;     // this is the horizontal distance
    float         advance_y    = 0.0f;     // this is the vertical distance
    float         s0           = 0.0f;     // First normalized texture coordinate (x) of top-left corner
    float         t0           = 0.0f;     // Second normalized texture coordinate (y) of top-left corner
    float         s1           = 0.0f;     // First normalized texture coordinate (x) of bottom-right corner
    float         t1           = 0.0f;     // Second normalized texture coordinate (y) of bottom-right corner
    std::int32_t  outline_type = 0;        // Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
    float         outline_thickness = 0;   // Glyph outline thickness

    std::map<std::uint32_t, float> kerning;   // key = left_charcode, kerning
};

class TexFont
{
public:
    enum class FontLocation
    {
        TEXTURE_FONT_FILE,
        TEXTURE_FONT_MEMORY
    };

    TexFont(float pt_size, std::string const & filename);
    TexFont(float pt_size, unsigned char const * memory_base, size_t memory_size);

    Glyph &      textureFontGetGlyph(const std::uint32_t ucodepoint);
    std::int32_t textureFontLoadGlyph(char const * charcode);
    std::int32_t textureFontLoadGlyph(std::uint32_t ucodepoint);

    size_t textureFontCacheGlyphs(char const * charcodes);

    AtlasTex & getAtlas() { return _atlas; }

    float glyphGetKerning(
        Glyph const &       glyph,
        const std::uint32_t left_charcode) const;   // charcode  codepoint of the peceding glyph
protected:
private:
    bool initFont();
    void textureFontGenerateKerning();
    void textureFontGenerateKerning(Glyph & glyph);

    void resizeAtlas();

    std::vector<Glyph> _glyphs;
    AtlasTex           _atlas;

    float         _size;                // Font size
    int           _hinting;             // Whether to use autohint when rendering font
    int           _outline_type;        // Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
    float         _outline_thickness;   // Outline thickness
    int           _filtering;           // Whether to use our own lcd filter
    int           _kerning;             // Whether to use kerning if available
    unsigned char _lcd_weights[5];      // LCD filter weights

    float _height;      // This field is simply used to compute a default line spacing (i.e., the
                        // baseline-to-baseline distance) when writing text with this font.
    float _linegap;     // This field is the distance that must be placed between two lines of text. The
                        // baseline-to-baseline distance should be computed as: ascender - descender + linegap
    float _ascender;    // The ascender is the vertical distance from the horizontal baseline to the highest
                        // 'character' coordinate in a font face.
    float _descender;   // The descender is the vertical distance from the horizontal baseline to the lowest
                        // 'character' coordinate in a font face.
    float _underline_position;    // The position of the underline line for this face.
    float _underline_thickness;   // The thickness of the underline for this face.

    FontLocation               _location;
    std::string                _filename;   // Font filename, for when location == TEXTURE_FONT_FILE
    std::vector<unsigned char> _memory;     // Font memory, for when location == TEXTURE_FONT_MEMORY
};

#endif   // TEXFONT_H
