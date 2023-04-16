#include "TextRender.h"
#include <cstring>

void AddText(VertexBuffer & vb, TexFont & tf, char const * text, glm::vec2 & pen)
{
    Glyph * prev_glyph = nullptr;
    for(unsigned int i = 0; i < std::strlen(text); i += utf8_surrogate_len(text + i))
    {
        std::uint32_t ucodepoint = utf8_to_utf32(text + i);
        Glyph &       glyph      = tf.textureFontGetGlyph(ucodepoint);

        float kerning = 0.0f;
        if(prev_glyph != nullptr)
        {
            kerning = tf.glyphGetKerning(glyph, prev_glyph->charcode);
        }
        prev_glyph = &glyph;

        pen.x += kerning;
        float        x0              = static_cast<int>(pen.x + glyph.offset_x);
        float        y0              = static_cast<int>(pen.y + glyph.offset_y);
        float        x1              = static_cast<int>(x0 + glyph.width);
        float        y1              = static_cast<int>(y0 - glyph.height);
        float        s0              = glyph.s0;
        float        t0              = glyph.t0;
        float        s1              = glyph.s1;
        float        t1              = glyph.t1;
        unsigned int indices[6]      = {0, 1, 2, 0, 2, 3};
        float        vertices[4 * 5] = {x0, y0, 0.0, s0, t0, x0, y1, 0.0, s0, t1,
                                        x1, y1, 0.0, s1, t1, x1, y0, 0.0, s1, t0};

        vb.VertexBufferPushBack(vertices, 4, indices, 6);
        pen.x += glyph.advance_x;
    }
}
