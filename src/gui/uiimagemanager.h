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
    glm::vec2 left_bottom = {};   // pixel coordinates
    glm::vec2 right_top   = {};
    glm::vec2 tx0         = {};   // normalized coordinates
    glm::vec2 tx1         = {};
    // nine slice data
    int32_t left   = 0;
    int32_t right  = 0;
    int32_t bottom = 0;
    int32_t top    = 0;

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
    UIImageGroup(UIImageManager & owner) : m_owner(owner) {}

    UIImageManager & getOwner() { return m_owner; }

    std::int32_t addImage(std::string name, std::string path, tex::ImageData const & image, int32_t left,
                          int32_t right, int32_t bottom, int32_t top);

    RegionDataOfUITexture const & getImageRegion(std::string const & name) const;

    void reloadImages();

private:
    UIImageManager &                   m_owner;
    std::vector<RegionDataOfUITexture> m_regions;
};

class UIImageManager
{
public:
    // json keys
    static constexpr char const * sid_gui_set        = "gui_sets";
    static constexpr char const * sid_set_name       = "set_name";
    static constexpr char const * sid_images         = "images";
    static constexpr char const * sid_texture        = "texture";
    static constexpr char const * sid_9slice_margins = "9slice_margins";

public:
    UIImageManager() = default;

    void                 parseUIRes(std::string const & file_name);
    UIImageGroup const & getImageGroup(std::string const & group_name) const;

    AtlasTex & getAtlas() { return m_atlas; }
    void       resizeAtlas();

private:
    using image_group_map = std::map<std::string, std::unique_ptr<UIImageGroup>>;

    AtlasTex        m_atlas;   // one tex atlas for all loaded UI elements
    image_group_map m_groups;
};

#endif
