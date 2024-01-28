#include "atlastex.h"
#include "imagedata.h"
#include <cassert>
#include <climits>
#include <cstring>

#include <GL/glew.h>

AtlasTex::AtlasTex(uint32_t size)
    : m_size{size}
{
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    m_nodes.emplace_back(1, 1, m_size - 2);
    m_data.resize(m_size * m_size * 4);
    std::memset(m_data.data(), 0, m_size * m_size * 4);
}

void AtlasTex::clear()
{
    m_nodes.clear();

    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    m_nodes.emplace_back(1, 1, m_size - 2);
    m_data.resize(m_size * m_size * 4);
    std::memset(m_data.data(), 0, m_size * m_size * 4);
}

int32_t AtlasTex::atlasFit(uint32_t index, uint32_t width, uint32_t height)
{
    glm::ivec3 * node;
    int32_t      x, y, width_left;
    size_t       i;

    node       = &m_nodes[index];
    x          = node->x;
    y          = node->y;
    width_left = width;
    i          = index;

    if((x + width) > (m_size - 1))
    {
        return -1;
    }

    while(width_left > 0)
    {
        node = &m_nodes[i];

        if(node->y > y)
        {
            y = node->y;
        }

        if((y + height) > (m_size - 1))
        {
            return -1;
        }

        width_left -= node->z;
        ++i;
    }

    return y;
}

void AtlasTex::atlasMerge()
{
    glm::ivec3 *node, *next;

    for(auto it = std::begin(m_nodes); it < std::end(m_nodes) - 1; ++it)
    {
        node = &(*it);
        next = &(*(it + 1));

        if(node->y == next->y)
        {
            node->z += next->z;
            it       = m_nodes.erase(it + 1);
        }
    }
}

glm::ivec4 AtlasTex::getRegion(uint32_t width, uint32_t height)
{
    int32_t     y, best_height, best_width, best_index;
    glm::ivec3 *node, *prev;
    glm::ivec4  region(0, 0, width, height);
    size_t      i;
    best_height = std::numeric_limits<int32_t>::max();
    best_index  = -1;
    best_width  = std::numeric_limits<int32_t>::max();

    for(i = 0; i < m_nodes.size(); ++i)
    {
        y = atlasFit(i, width, height);

        if(y >= 0)
        {
            node = &m_nodes[i];

            if(((y + static_cast<int32_t>(height)) < best_height)
               || (((y + static_cast<int32_t>(height)) == best_height) && (node->z < best_width)))
            {
                best_height = y + height;
                best_index  = i;
                best_width  = node->z;
                region.x    = node->x;
                region.y    = y;
            }
        }
    }

    if(best_index == -1)
    {
        region.x = -1;
        region.y = -1;
        region.z = 0;
        region.w = 0;
        return region;
    }

    glm::ivec3 nnode;
    nnode.x = region.x;
    nnode.y = region.y + height;
    nnode.z = width;
    m_nodes.insert(std::begin(m_nodes) + best_index, std::move(nnode));

    for(i = best_index + 1; i < m_nodes.size(); ++i)
    {
        node = &m_nodes[i];
        prev = &m_nodes[i - 1];

        if(node->x < (prev->x + prev->z))
        {
            int32_t shrink  = prev->x + prev->z - node->x;
            node->x        += shrink;
            node->z        -= shrink;

            if(node->z <= 0)
            {
                m_nodes.erase(std::begin(m_nodes) + i);
                --i;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    atlasMerge();
    return region;
}

void AtlasTex::setRegionTL(glm::ivec4 reg, unsigned char const * data, int32_t stride,
                           int32_t bytes_ppx)   // z - width, w - height
{
    assert(reg.x > 0);
    assert(reg.y > 0);
    assert(reg.x < (static_cast<int32_t>(m_size) - 1));
    assert((reg.x + reg.z) <= (static_cast<int32_t>(m_size) - 1));
    assert(reg.y < (static_cast<int32_t>(m_size) - 1));
    assert((reg.y + reg.w) <= (static_cast<int32_t>(m_size) - 1));

    int32_t charsize = sizeof(char);
    int32_t y        = reg.y + reg.w;

    for(int32_t i = 0; i < reg.w; ++i)
    {
        unsigned char bytes[4] = {0};   // 4 - alpha

        for(int32_t j = 0; j < reg.z; ++j)
        {
            uint32_t dst_shift = (((y - i) * m_size + reg.x + j) * charsize * 4);
            uint32_t src_shift = ((i * stride) + j * bytes_ppx * charsize);

            bytes[0] = data[src_shift + 0];
            bytes[1] = data[src_shift + 1];
            bytes[2] = data[src_shift + 2];
            if(bytes_ppx == 3)
                bytes[3] = std::min(bytes[0] + bytes[1] + bytes[2], 255);
            else
                bytes[3] = data[src_shift + 3];

            m_data[dst_shift + 0] = bytes[0];
            m_data[dst_shift + 1] = bytes[1];
            m_data[dst_shift + 2] = bytes[2];
            m_data[dst_shift + 3] = bytes[3];
        }
    }
}

void AtlasTex::setRegionBL(glm::ivec4 reg, unsigned char const * data, int32_t stride, int32_t bytes_ppx)
{
    assert(reg.x > 0);
    assert(reg.y > 0);
    assert(reg.x < (static_cast<int32_t>(m_size) - 1));
    assert((reg.x + reg.z) <= (static_cast<int32_t>(m_size) - 1));
    assert(reg.y < (static_cast<int32_t>(m_size) - 1));
    assert((reg.y + reg.w) <= (static_cast<int32_t>(m_size) - 1));

    uint32_t charsize = sizeof(char);

    for(int32_t i = 0; i < reg.w; ++i)
    {
        unsigned char bytes[4] = {0};   // 4 - alpha

        for(int32_t j = 0; j < reg.z; ++j)
        {
            uint32_t dst_shift = (((reg.y + i) * m_size + reg.x + j) * charsize * 4);
            uint32_t src_shift = ((i * stride) + j * bytes_ppx * charsize);

            bytes[0] = data[src_shift + 0];
            bytes[1] = data[src_shift + 1];
            bytes[2] = data[src_shift + 2];
            if(bytes_ppx == 3)
                bytes[3] = std::min(bytes[0] + bytes[1] + bytes[2], 255);
            else
                bytes[3] = data[src_shift + 3];

            m_data[dst_shift + 0] = bytes[0];
            m_data[dst_shift + 1] = bytes[1];
            m_data[dst_shift + 2] = bytes[2];
            m_data[dst_shift + 3] = bytes[3];
        }
    }
}

void AtlasTex::writeAtlasToTGA(std::string const & name)
{
    tex::ImageData image;
    image.height = m_size;
    image.width  = m_size;
    image.type   = tex::ImageData::PixelType::pt_rgba;
    image.data   = std::make_unique<uint8_t[]>(m_data.size());

    std::memcpy(image.data.get(), m_data.data(), m_data.size());

    tex::WriteTGA(name, image);
}

////////////////////////
void AtlasTex::BindTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_atlas_tex_id);
}

void AtlasTex::UploadTexture()
{
    glGenTextures(1, &m_atlas_tex_id);

    glBindTexture(GL_TEXTURE_2D, m_atlas_tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getSize(), getSize(), 0, GL_RGBA, GL_UNSIGNED_BYTE, getData());
}

void AtlasTex::DeleteTexture()
{
    glDeleteTextures(1, &m_atlas_tex_id);
}
