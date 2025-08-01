#include "uiimagemanager.h"
#include <algorithm>
#include <stdexcept>

#include "src/render/vertex_buffer.h"

void RegionDataOfUITexture::addBlock(VertexBuffer & vb, glm::vec2 & pos, glm::vec2 new_size) const
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
    float const inv_new_width    = 1.f / new_size.x;
    float const inv_new_height   = 1.f / new_size.y;
    float const tex_coord_width  = tx1.s - tx0.s;
    float const tex_coord_height = tx1.t - tx0.t;

    float const x0 = pos.x;
    float const y0 = pos.y;
    float const s0 = tx0.s;
    float const t0 = tx0.t;

    float const x1 = x0 + static_cast<float>(left);
    float const y1 = y0 + static_cast<float>(bottom);
    float const s1 = s0 + inv_new_width * static_cast<float>(left) * tex_coord_width;
    float const t1 = t0 + inv_new_height * static_cast<float>(bottom) * tex_coord_height;

    float const x3 = x0 + new_size.x;
    float const y3 = y0 + new_size.y;
    float const s3 = tx1.s;
    float const t3 = tx1.t;

    float const x2 = x3 - static_cast<float>(right);
    float const y2 = y3 - static_cast<float>(top);
    float const s2 = s0 + inv_new_width * (new_size.x - static_cast<float>(right)) * tex_coord_width;
    float const t2 = t0 + inv_new_height * (new_size.y - static_cast<float>(top)) * tex_coord_height;

    // add 9 rectangles to the vertex buffer
    // bottom row
    Add2DRectangle(vb, x0, y0, x1, y1, s0, t0, s1, t1);
    Add2DRectangle(vb, x1, y0, x2, y1, s1, t0, s2, t1);
    Add2DRectangle(vb, x2, y0, x3, y1, s2, t0, s3, t1);

    // middle row
    Add2DRectangle(vb, x0, y1, x1, y2, s0, t1, s1, t2);
    Add2DRectangle(vb, x1, y1, x2, y2, s1, t1, s2, t2);
    Add2DRectangle(vb, x2, y1, x3, y2, s2, t1, s3, t2);

    // top row
    Add2DRectangle(vb, x0, y2, x1, y3, s0, t2, s1, t3);
    Add2DRectangle(vb, x1, y2, x2, y3, s1, t2, s2, t3);
    Add2DRectangle(vb, x2, y2, x3, y3, s2, t2, s3, t3);

    // move pen position
    pos.x += new_size.x;
}

UIImageGroup const & UIImageGroupManager::getImageGroup(std::string const & group_name) const
{
    if(auto search = m_groups.find(group_name); search != m_groups.end())
        return *search->second;

    throw std::runtime_error("ImageGroup with requested name not found");
}

void UIImageGroupManager::resizeAtlas()
{
    AtlasTex new_atlas(m_atlas.getSize() * 2);
    m_atlas = std::move(new_atlas);

    for(auto & gr: m_groups)
    {
        gr.second->reloadImages();
    }
}

int32_t UIImageGroup::addImage(std::string name, std::string path, tex::ImageData const & image, int32_t left,
                               int32_t right, int32_t bottom, int32_t top)
{
    glm::ivec4 region;
    size_t     x, y, w, h;
    size_t     size            = m_owner.getAtlas().getSize();
    float      inv_size        = 1.0f / static_cast<float>(size);
    int32_t    bytes_per_pixel = image.type == tex::ImageData::PixelType::pt_rgb ? 3 : 4;

    w      = image.width + 1;
    h      = image.height + 1;
    region = m_owner.getAtlas().getRegion(w, h);

    if(region.x < 0)
    {
        return -1;
    }

    w = w - 1;
    h = h - 1;
    x = region.x;
    y = region.y;
    m_owner.getAtlas().setRegionBL(glm::ivec4(x, y, w, h), image.data.get(), image.width, bytes_per_pixel);

    RegionDataOfUITexture tex_region;
    tex_region.name        = std::move(name);
    tex_region.path        = std::move(path);
    tex_region.left_bottom = glm::vec2(x, y);
    tex_region.right_top   = glm::vec2(x + w, y + h);
    tex_region.tx0.s       = x * inv_size;
    tex_region.tx0.t       = y * inv_size;
    tex_region.tx1.s       = (x + w) * inv_size;
    tex_region.tx1.t       = (y + h) * inv_size;
    tex_region.left        = left;
    tex_region.right       = right;
    tex_region.bottom      = bottom;
    tex_region.top         = top;

    m_regions.push_back(tex_region);

    return m_regions.size() - 1;
}

RegionDataOfUITexture const * UIImageGroup::getImageRegion(std::string const & name) const
{
    auto it = std::find_if(begin(m_regions), end(m_regions),
                           [&name](auto & region) { return name == region.name; });

    if(it != m_regions.end())
        return &(*it);

    return nullptr;
}

void UIImageGroup::reloadImages()
{
    auto regions = std::move(m_regions);
    m_regions.clear();

    for(auto & reg: regions)
    {
        tex::ImageData image;
        if(auto file = m_fsys.getFile(reg.path); file)
        {
            if(!tex::ReadTGA(*file, image))
                continue;
        }
        else
            continue;
        // we don't care about the oversize atlas error in this function
        addImage(reg.name, reg.path, image, reg.left, reg.right, reg.bottom, reg.top);
    }
}
