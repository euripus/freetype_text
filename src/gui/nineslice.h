#ifndef NINESLICE_H
#define NINESLICE_H

#include "src/VertexBuffer.h"
#include <glm/glm.hpp>

//                      (right_top, tx1)
//  --------------------
//  |                  |
//  |                  |
//  |                  |
//  |                  |
//  --------------------
// (left_bottom, tx0)
struct RegionDataOfUITexture
{
    glm::vec2 left_bottom;   // pixel coordinates
    glm::vec2 right_top;
    glm::vec2 tx0;   // normalized coordinates
    glm::vec2 tx1;

    float     getWidth() const { return right_top.x - left_bottom.x; }
    float     getHeight() const { return right_top.y - left_bottom.y; }
    glm::vec2 getSize() const { return {getWidth(), getHeight()}; }
};

struct NineSliceBlock
{
    int32_t               left;
    int32_t               right;
    int32_t               bottom;
    int32_t               top;
    RegionDataOfUITexture origin;

    void addBlock(VertexBuffer & vb, glm::vec2 & pos, glm::vec2 new_size) const;
};

#endif
