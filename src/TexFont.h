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
    std::uint32_t outline_type = 0;        // Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
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

    AtlasTex & getAtlas() { return m_atlas; }

    float glyphGetKerning(
        Glyph const &       glyph,
        const std::uint32_t left_charcode) const;   // charcode  codepoint of the peceding glyph

    glm::vec2 getTextSize(char const * text);
protected:
private:
    bool initFont();
    void textureFontGenerateKerning();
    void textureFontGenerateKerning(Glyph & glyph);

    void resizeAtlas();

    std::vector<Glyph> m_glyphs;
    AtlasTex           m_atlas;

    float         m_size;                // Font size
    int           m_hinting;             // Whether to use autohint when rendering font
    std::uint32_t m_outline_type;        // Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
    float         m_outline_thickness;   // Outline thickness
    int           m_filtering;           // Whether to use our own lcd filter
    int           m_kerning;             // Whether to use kerning if available
    unsigned char m_lcd_weights[5];      // LCD filter weights

    float m_height;     // This field is simply used to compute a default line spacing (i.e., the
                        // baseline-to-baseline distance) when writing text with this font.
    float m_linegap;    // This field is the distance that must be placed between two lines of text. The
                        // baseline-to-baseline distance should be computed as: ascender - descender + linegap
    float m_ascender;   // The ascender is the vertical distance from the horizontal baseline to the highest
                        // 'character' coordinate in a font face.
    float m_descender;   // The descender is the vertical distance from the horizontal baseline to the lowest
                         // 'character' coordinate in a font face.
    float m_underline_position;    // The position of the underline line for this face.
    float m_underline_thickness;   // The thickness of the underline for this face.

    FontLocation               m_location;
    std::string                m_filename;   // Font filename, for when location == TEXTURE_FONT_FILE
    std::vector<unsigned char> m_memory;     // Font memory, for when location == TEXTURE_FONT_MEMORY
};

#endif   // TEXFONT_H
