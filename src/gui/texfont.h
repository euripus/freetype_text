#ifndef TEXFONT_H
#define TEXFONT_H

#include <map>
#include <string>
#include <vector>
#include <glm/glm.hpp>

//  Glyph metrics:
//  --------------
//                        xmin                     xmax
//                         |                         |
//                         |<-------- width -------->|
//                         |                         |
//               |         +-------------------------+----------------- ymax
//               |         |    ggggggggg   ggggg    |     ^        ^
//               |         |   g:::::::::ggg::::g    |     |        |
//               |         |  g:::::::::::::::::g    |     |        |
//               |         | g::::::ggggg::::::gg    |     |        |
//               |         | g:::::g     g:::::g     |     |        |
//     offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
//               |         | g:::::g     g:::::g     |     |        |
//               |         | g::::::g    g:::::g     |     |        |
//               |         | g:::::::ggggg:::::g     |     |        |
//               |         |  g::::::::::::::::g     |     |      height
//               |         |   gg::::::::::::::g     |     |        |
//   baseline ---*---------|---- gggggggg::::::g-----*--------      |
//             / |         |             g:::::g     |              |
//      origin   |         | gggggg      g:::::g     |              |
//               |         | g:::::gg   gg:::::g     |              |
//               |         |  g::::::ggg:::::::g     |              |
//               |         |   gg:::::::::::::g      |              |
//               |         |     ggg::::::ggg        |              |
//               |         |         gggggg          |              v
//               |         +-------------------------+----------------- ymin
//               |                                   |
//               |------------- advance_x ---------->|

struct Glyph
{
    enum class OutlineType
    {
        NONE,
        LINE,
        INNER,
        OUTER
    };

    std::uint32_t charcode     = -1;     // Wide character this glyph represents
    size_t        width        = 0;      // Glyph's width in pixels
    size_t        height       = 0;      // Glyph's height in pixels.
    int32_t       offset_x     = 0;      // Glyph's left bearing expressed in integer pixels.
    int32_t       offset_y     = 0;      // Glyphs's top bearing expressed in integer pixels.
    float         advance_x    = 0.0f;   // this is the horizontal distance
    float         advance_y    = 0.0f;   // this is the vertical distance
    float         s0           = 0.0f;   // First normalized texture coordinate (x) of bottom-left corner
    float         t0           = 0.0f;   // Second normalized texture coordinate (y) of bottom-left corner
    float         s1           = 0.0f;   // First normalized texture coordinate (x) of top-right corner
    float         t1           = 0.0f;   // Second normalized texture coordinate (y) of top-right corner
    OutlineType   outline_type = OutlineType::NONE;   // Glyph outline type
    float         outline_thickness = 0;              // Glyph outline thickness

    std::map<std::uint32_t, float> kerning;   // key = left_charcode, kerning
};

class FontManager;
class VertexBuffer;

class TexFont
{
public:
    enum class FontLocation
    {
        FONT_FILE,
        FONT_MEMORY
    };

    enum class RenderMode
    {
        LCD,
        NORMAL
    };

    TexFont(FontManager & owner, std::string const & filename, float pt_size, bool hinting = true,
            bool kerning = true, float outline_thickness = 0.0f,
            Glyph::OutlineType outline_type = Glyph::OutlineType::NONE, RenderMode mode = RenderMode::NORMAL);
    TexFont(FontManager & owner, unsigned char const * memory_base, size_t memory_size, float pt_size,
            bool hinting = true, bool kerning = true, float outline_thickness = 0.0f,
            Glyph::OutlineType outline_type = Glyph::OutlineType::NONE, RenderMode mode = RenderMode::NORMAL);

    Glyph const & getGlyph(std::uint32_t const ucodepoint) const;
    std::int32_t  loadGlyph(char const * charcode);
    std::int32_t  loadGlyph(std::uint32_t ucodepoint);

    size_t cacheGlyphs(char const * charcodes);

    float glyphGetKerning(
        Glyph const &       glyph,
        std::uint32_t const left_charcode) const;   // charcode  codepoint of the peceding glyph

    glm::vec2 getTextSize(char const * text) const;
    void      addText(VertexBuffer & vb, char const * text, glm::vec2 & pos) const;
    void      addGlyph(VertexBuffer & vb, std::uint32_t ucodepoint, Glyph const * prev_glyph,
                       glm::vec2 & pos) const;

    void reloadGlyphs();

    float      getHeight() const { return m_height; }
    float      getSize() const { return m_size; }
    float      getLineGap() const { return m_linegap; }
    float      getAscender() const { return m_ascender; }
    float      getDescender() const { return m_descender; }
    RenderMode getRenderMode() const { return m_render_mode; }

private:
    bool initFont();
    void generateKerning();
    void generateKerning(Glyph & glyph);

    FontManager & m_owner;

    std::vector<Glyph> m_glyphs;

    float              m_size;                // Font size
    bool               m_hinting;             // Whether to use autohint when rendering font
    Glyph::OutlineType m_outline_type;        // Outline type
    float              m_outline_thickness;   // Outline thickness
    bool               m_kerning;             // Whether to use kerning if available
    unsigned char      m_lcd_weights[5];      // LCD filter weights

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

    RenderMode                 m_render_mode;
    FontLocation               m_location;
    std::string                m_filename;   // Font filename, for when location == TEXTURE_FONT_FILE
    std::vector<unsigned char> m_memory;     // Font memory, for when location == TEXTURE_FONT_MEMORY

    friend struct MarkupText;
};

struct MarkupText
{
    // TODO background color
    enum class LineType
    {
        UNDERLINE,
        OVERLINE,
        STRIKETHROUGH
    };

    MarkupText(TexFont & font, LineType line = LineType::UNDERLINE)
        : m_font(font),
          m_line(line)
    {}

    void addText(VertexBuffer & vb, char const * text, glm::vec2 & pos) const;
    void addGlyph(VertexBuffer & vb, std::uint32_t ucodepoint, Glyph const * prev_glyph,
                  glm::vec2 & pos) const;

    TexFont & m_font;
    LineType  m_line;
};

#endif   // TEXFONT_H
