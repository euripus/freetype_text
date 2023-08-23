#ifndef UIIMAGEMANAGER_H
#define UIIMAGEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <glm/glm.hpp>

#include "src/gui/atlastex.h"
#include "src/gui/imagedata.h"

class UIImageManager;
class VertexBuffer;

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
    // nine slice data
    int32_t left;
    int32_t right;
    int32_t bottom;
    int32_t top;
	
	std::string name;
	std::string path;

    float     getWidth() const { return right_top.x - left_bottom.x; }
    float     getHeight() const { return right_top.y - left_bottom.y; }
    glm::vec2 getSize() const { return {getWidth(), getHeight()}; }

    void addBlock(VertexBuffer & vb, glm::vec2 & pos, glm::vec2 new_size) const;
};

class UIImageGroup   // a group of images of the same style
{
public:
    UIImageGroup(UIImageManager & owner, std::string name) : m_owner(owner), m_name(std::move(name)) {}

    std::int32_t addImage(std::string name, std::string path, tex::ImageData const & image, int32_t left, int32_t right,
                           int32_t bottom, int32_t top);

    RegionDataOfUITexture const & getImageRegion(std::string const & name) const;

    void reloadImages();

private:
    using region_data = std::pair<std::string, RegionDataOfUITexture>;

    UIImageManager &         m_owner;
    std::string              m_name;
    std::vector<region_data> m_regions;
};

class UIImageManager
{
public:
    UIImageManager() = default;

    UIImageGroup &       addImageGroup(std::string const & group_name);
    UIImageGroup const & getImageGroup(std::string const & group_name) const;

    AtlasTex & getAtlas() { return m_atlas; }
    void       resizeAtlas();

private:
    using image_group_map = std::map<std::string, std::unique_ptr<UIImageGroup>>;

    AtlasTex        m_atlas;   // one tex atlas for all loaded UI elements
    image_group_map m_groups;
};

#endif
