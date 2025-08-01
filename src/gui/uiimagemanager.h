#ifndef UIIMAGEMANAGER_H
#define UIIMAGEMANAGER_H

#include <vector>
#include <map>
#include <memory>
#include <glm/glm.hpp>

#include "src/gui/atlastex.h"
#include "src/res/imagedata.h"
#include "src/fs/file_system.h"

class UIImageGroupManager;
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
    UIImageGroup(UIImageGroupManager & owner, FileSystem & fsys)
        : m_owner(owner),
          m_fsys(fsys)
    {}

    UIImageGroupManager & getOwner() { return m_owner; }

    int32_t addImage(std::string name, std::string path, tex::ImageData const & image, int32_t left,
                     int32_t right, int32_t bottom, int32_t top);

    RegionDataOfUITexture const * getImageRegion(std::string const & name) const;

    void reloadImages();

private:
    UIImageGroupManager &              m_owner;
    FileSystem &                       m_fsys;
    std::vector<RegionDataOfUITexture> m_regions;
};

class UIImageGroupManager
{
public:
    UIImageGroupManager() = default;

    UIImageGroup const & getImageGroup(std::string const & group_name) const;

    AtlasTex & getAtlas() { return m_atlas; }
    void       resizeAtlas();

private:
    using image_group_map = std::map<std::string, std::unique_ptr<UIImageGroup>>;

    AtlasTex        m_atlas;   // one tex atlas for all loaded UI elements
    image_group_map m_groups;

    friend class UIImageManagerDesc;
};

#endif
