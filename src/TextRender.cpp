#include "TextRender.h"

void AddText(VertexBuffer & vb, TexFont & tf, wchar_t * text, glm::vec2 & pen)
{
    for(unsigned int i = 0; i < wcslen(text); ++i)
    {
        Glyph & glyph = tf.TextureFontGetGlyph(text[i]);

        float kerning = 0;   // !!!!!!!!!!!!!!!!!!!!!!!
        if(i > 0)
        {
            kerning = tf.GlyphGetKerning(glyph, text[i - 1]);
        }

        pen.x += kerning;
        float        x0              = (int)(pen.x + glyph.offset_x);
        float        y0              = (int)(pen.y + glyph.offset_y);
        float        x1              = (int)(x0 + glyph.width);
        float        y1              = (int)(y0 - glyph.height);
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
