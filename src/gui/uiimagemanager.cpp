#include "uiimagemanager.h"
#include <fstream>
#include <boost/json.hpp>
#include <algorithm>

#include "src/vertex_buffer.h"

void parseImages(boost::json::value const & jv, UIImageGroup & group);

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
    float inv_new_width    = 1.0f / new_size.x;
    float inv_new_height   = 1.0f / new_size.y;
    float tex_coord_width  = tx1.s - tx0.s;
    float tex_coord_height = tx1.t - tx0.t;

    float x0 = pos.x;
    float y0 = pos.y;
    float s0 = tx0.s;
    float t0 = tx0.t;

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
    float s3 = tx1.s;
    float t3 = tx1.t;

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

void UIImageManager::parseUIRes(std::string const & file_name)
{
    boost::json::value jv;

    try
    {
        std::ifstream ifile(file_name, std::ios::in);
        std::string   file_data;

        if(ifile.is_open())
        {
            std::string tp;
            while(std::getline(ifile, tp))
            {
                file_data += tp;
            }
        }
        else
        {
            std::string err = "File: " + file_name + " - not found!";
            throw std::runtime_error(err);
        }

        jv = boost::json::parse(file_data);
    }
    catch(std::exception const & e)
    {
        throw std::runtime_error(e.what());
    }

    assert(!jv.is_null());

    auto const & obj        = jv.get_object();
    auto const   gui_set_it = obj.find(sid_gui_set);
    if(gui_set_it != obj.end())
    {
        auto const & arr = gui_set_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & set_val: arr)
            {
                std::string gr_name;

                auto const & array_obj = set_val.as_object();
                for(auto const & kvp: array_obj)
                {
                    if(kvp.key() == sid_set_name)
                    {
                        gr_name = kvp.value().as_string();
                    }
                    else if(kvp.key() == sid_images)
                    {
                        m_groups[gr_name] = std::make_unique<UIImageGroup>(*this);
                        parseImages(kvp.value(), *m_groups[gr_name]);
                    }
                }
            }
        }
    }
    else
    {
        std::string err = "In file " + file_name + " not found " + sid_gui_set;
        throw std::runtime_error(err);
    }
}

void parseImages(boost::json::value const & jv, UIImageGroup & group)
{
    auto const & arr = jv.get_array();
    if(!arr.empty())
    {
        for(auto const & kvp: arr)
        {
            std::string          path;
            std::string          name;
            std::vector<int32_t> margins;

            auto const it = kvp.get_object().begin();
            name          = it->key();

            for(auto const & kvp2: it->value().as_object())
            {
                if(kvp2.key() == UIImageManager::sid_texture)
                    path = kvp2.value().as_string();
                else if(kvp2.key() == UIImageManager::sid_9slice_margins)
                {
                    margins = boost::json::value_to<std::vector<int32_t>>(kvp2.value());
                }
            }

            tex::ImageData image;
            if(!tex::ReadTGA(path, image))
                continue;

            if(group.addImage(name, path, image, margins[0], margins[1], margins[2], margins[3]) == -1)
            {
                // texture atlas is full
                // let's try again
                group.getOwner().resizeAtlas();
                if(group.addImage(name, path, image, margins[0], margins[1], margins[2], margins[3]) == -1)
                    throw std::runtime_error("Texture atlas is full");
            }
        }
    }
}

UIImageGroup const & UIImageManager::getImageGroup(std::string const & group_name) const
{
    if(auto search = m_groups.find(group_name); search != m_groups.end())
        return *search->second;

    throw std::runtime_error("ImageGroup with requested name not found");
}

void UIImageManager::resizeAtlas()
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
    m_owner.getAtlas().setRegionBL(glm::ivec4(x, y, w, h), image.data.get(), image.width * bytes_per_pixel,
                                   bytes_per_pixel);

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
        if(!tex::ReadTGA(reg.path, image))
            continue;
        // we don't care about the oversize atlas error in this function
        addImage(reg.name, reg.path, image, reg.left, reg.right, reg.bottom, reg.top);
    }
}
