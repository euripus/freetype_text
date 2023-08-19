#ifndef UIIMAGEMANAGER_H
#define UIIMAGEMANAGER_H

#include <vector>
#include <glm/glm.hpp>

#include "src/gui/atlastex.h"
#include "src/gui/imagedata.h"

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
    std::uint32_t id;

    glm::vec2 left_bottom;   // pixel coordinates
    glm::vec2 right_top;
    glm::vec2 tx0;   // normalized coordinates
    glm::vec2 tx1;

    float     getWidth() const { return right_top.x - left_bottom.x; }
    float     getHeight() const { return right_top.y - left_bottom.y; }
    glm::vec2 getSize() const { return {getWidth(), getHeight()}; }
};

class UIImageManager
{
public:
    UIImageManager() = default;

    void addImageData(tex::ImageData const & image);
private:
    std::vector<RegionDataOfUITexture> m_regions;
    AtlasTex                           m_atlas;   // one tex atlas for all loaded UI elements
};

#endif
