#include "AtlasTex.h"
#include "Tga.h"
#include <cassert>
#include <climits>
#include <cstring>

void AtlasTex::Delete()
{
    _nodes.clear();
    _data.clear();

    _width  = 0;
    _height = 0;
    _used   = 0;
}

void AtlasTex::Clear()
{
    glm::ivec3 node(1, 1, 1);
    _nodes.clear();
    _used = 0;
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    node.z = _width - 2;
    _nodes.push_back(std::move(node));
    _data.resize(_width * _height * 4);
    std::memset(_data.data(), 0, _width * _height * 4);
}

void AtlasTex::Create(unsigned int width, unsigned int height)
{
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    glm::ivec3 node(1, 1, width - 2);

    _used   = 0;
    _width  = width;
    _height = height;
    _nodes.push_back(std::move(node));
    _data.resize(_width * _height * 4);
    std::memset(_data.data(), 0, _width * _height * 4);
}

int AtlasTex::AtlasFit(unsigned int index, unsigned int width, unsigned int height)
{
    glm::ivec3 * node;
    int          x, y, width_left;
    size_t       i;
    node       = &_nodes[index];
    x          = node->x;
    y          = node->y;
    width_left = width;
    i          = index;

    if((x + width) > (_width - 1))
    {
        return -1;
    }

    while(width_left > 0)
    {
        node = &_nodes[i];

        if(node->y > y)
        {
            y = node->y;
        }

        if((y + height) > (_height - 1))
        {
            return -1;
        }

        width_left -= node->z;
        ++i;
    }

    return y;
}

void AtlasTex::AtlasMerge()
{
    glm::ivec3 *node, *next;

    for(auto it = std::begin(_nodes); it < std::end(_nodes) - 1; ++it)
    {
        node = &(*it);
        next = &(*(it + 1));

        if(node->y == next->y)
        {
            node->z += next->z;
            _nodes.erase(it + 1);
            --it;
        }
    }
}

glm::ivec4 AtlasTex::GetRegion(unsigned int width, unsigned int height)
{
    int         y, best_height, best_width, best_index;
    glm::ivec3 *node, *prev;
    glm::ivec4  region(0, 0, width, height);
    size_t      i;
    best_height = INT_MAX;
    best_index  = -1;
    best_width  = INT_MAX;

    for(i = 0; i < _nodes.size(); ++i)
    {
        y = AtlasFit(i, width, height);

        if(y >= 0)
        {
            node = &_nodes[i];

            if(((y + static_cast<int>(height)) < best_height)
               || (((y + static_cast<int>(height)) == best_height) && (node->z < best_width)))
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
    _nodes.insert(std::begin(_nodes) + best_index, std::move(nnode));

    for(i = best_index + 1; i < _nodes.size(); ++i)
    {
        node = &_nodes[i];
        prev = &_nodes[i - 1];

        if(node->x < (prev->x + prev->z))
        {
            int shrink = prev->x + prev->z - node->x;
            node->x += shrink;
            node->z -= shrink;

            if(node->z <= 0)
            {
                _nodes.erase(std::begin(_nodes) + i);
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

    AtlasMerge();
    _used += width * height;
    return region;
}

void AtlasTex::SetRegion(glm::ivec4 reg, unsigned char const * data, int stride)   // z - width, w - height
{
    assert(reg.x > 0);
    assert(reg.y > 0);
    assert(reg.x < (static_cast<int>(_width) - 1));
    assert((reg.x + reg.z) <= (static_cast<int>(_width) - 1));
    assert(reg.y < (static_cast<int>(_height) - 1));
    assert((reg.y + reg.w) <= (static_cast<int>(_height) - 1));

    int          i;
    unsigned int charsize;
    charsize = sizeof(char);

    for(i = 0; i < reg.w; ++i)
    {
        unsigned char bytes[4] = {0};   // 4 - alpha

        for(int j = 0; j < reg.z; ++j)
        {
            unsigned int dst_shift = (((reg.y + i) * _width + reg.x + j) * charsize * 4);
            unsigned int src_shift = (((i * stride) + j * 3) * charsize);

            bytes[0] = data[src_shift + 0];
            bytes[1] = data[src_shift + 1];
            bytes[2] = data[src_shift + 2];
            bytes[3] = std::min(bytes[0] + bytes[1] + bytes[2], 255);

            _data[dst_shift + 0] = bytes[0];
            _data[dst_shift + 1] = bytes[1];
            _data[dst_shift + 2] = bytes[2];
            _data[dst_shift + 3] = bytes[3];
        }
    }
}

void AtlasTex::WriteAtlasToTGA(std::string const & name)
{
    WriteUncompressedTGA(name.c_str(), 4, _width, _height, _data.data());
}

////////////////////////
GLuint atlTex;

void AtlasTex::BindTexture()
{
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, atlTex);
}

#define GL_CLAMP_TO_EDGE 0x812F

void AtlasTex::UploadTexture()
{
    glGenTextures(1, &atlTex);

    glBindTexture(GL_TEXTURE_2D, atlTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data.data());
}

void AtlasTex::DeleteTexture()
{
    glDeleteTextures(1, &atlTex);
}
