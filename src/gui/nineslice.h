#ifndef NINESLICE_H
#define NINESLICE_H

#include "src/VertexBuffer.h"
#include "src/gui/uiimagemanager.h"

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
