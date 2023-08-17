#include "nineslice.h"

void NineSliceBlock::addBlock(VertexBuffer & vb, glm::vec2 & pos, glm::vec2 new_size) const
{
    if(new_size.x < static_cast<float>(left + right) || new_size.y < static_cast<float>(bottom + top))
    {
        // Error: the new rectangle size is too small
        return;
    }

    //     L              R
    //  ---------------------- 3
    //  |  |              |  |
    //  |  |              |2 |
    //  |--------------------| T
    //  |  |              |  |
    //  |  |              |  |
    //  |  |1             |  |
    //  |--------------------| B
    //  |  |              |  |
    //  |  |              |  |
    // 0----------------------

    float inv_new_width    = 1.0f / new_size.x;
    float inv_new_height   = 1.0f / new_size.y;
    float tex_coord_width  = origin.tx1.s - origin.tx0.s;
    float tex_coord_height = origin.tx1.t - origin.tx0.t;

    float x0 = pos.x;
    float y0 = pos.y;
    float s0 = origin.tx0.s;
    float t0 = origin.tx0.t;

    float x1 = x0 + static_cast<float>(left);
    float y1 = y0 + static_cast<float>(bottom);
    float s1 = s0 + inv_new_width * x1 * tex_coord_width;
    float t1 = t0 + inv_new_height * y1 * tex_coord_height;

    float x2 = new_size.x - static_cast<float>(right);
    float y2 = new_size.y - static_cast<float>(top);
    float s2 = s0 + inv_new_width * x2 * tex_coord_width;
    float t2 = t0 + inv_new_height * y2 * tex_coord_height;

    float x3 = x0 + new_size.x;
    float y3 = y0 + new_size.y;
    float s3 = origin.tx1.s;
    float t3 = origin.tx1.t;

    // add 9 rectangles to the vertex buffer
    // bottom row
    add2DRectangle(vb, x0, y0, x1, y1, s0, t0, s1, t1);
    add2DRectangle(vb, x1, y0, x2, y1, s1, t0, s2, t1);
    add2DRectangle(vb, x2, y0, x3, y1, s2, t0, s3, t1);

    // middle row
    add2DRectangle(vb, x0, y1, x1, y2, s0, t1, s1, t2);
    add2DRectangle(vb, x1, y1, x2, y2, s1, t1, s2, t2);
    add2DRectangle(vb, x2, y1, x3, y2, s2, t1, s3, t2);

    // top row
    add2DRectangle(vb, x0, y2, x1, y3, s0, t2, s1, t3);
    add2DRectangle(vb, x1, y2, x2, y3, s1, t2, s2, t3);
    add2DRectangle(vb, x2, y2, x3, y3, s2, t2, s3, t3);

    // move pen position
    pos.x += new_size.x;
}
