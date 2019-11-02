#include "AtlasTex.h"
#include "Tga.h"
#include <cassert>
#include <climits>
#include <cstring>

AtlasTex::AtlasTex() : _width(0), _height(0), _depth(0), _used(0), _data(nullptr) {}

AtlasTex::~AtlasTex()
{
    Delete();
}

void AtlasTex::Delete()
{
    _nodes.clear();

    if(_data != nullptr)
    {
        delete[] _data;
        _data = nullptr;
    }

    _width  = 0;
    _height = 0;
    _depth  = 0;
    _used   = 0;
}

void AtlasTex::Clear()
{
    glm::ivec3 node(1, 1, 1);
    assert(_data);
    _nodes.clear();
    _used = 0;
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    node.z = _width - 2;
    _nodes.push_back(std::move(node));
    std::memset(_data, 0, _width * _height * _depth);
}

void AtlasTex::Create(unsigned int width, unsigned int height, unsigned int depth)
{
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    glm::ivec3 node(1, 1, width - 2);
    assert((depth == 1) || (depth == 2) || (depth == 3) || (depth == 4));
    _used   = 0;
    _width  = width;
    _height = height;
    _depth  = depth;
    _nodes.push_back(std::move(node));
    _data = new unsigned char[width * height * depth];
    std::memset(_data, 0, _width * _height * _depth);
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

    y = node->y;

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

            if(((y + height) < best_height) || (((y + height) == best_height) && (node->z < best_width)))
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

void AtlasTex::SetRegion(glm::ivec4 reg, const unsigned char * data, int stride)   // z - width, w - height
{
    unsigned int i;
    unsigned int charsize;
    assert(reg.x > 0);
    assert(reg.y > 0);
    assert(reg.x < (_width - 1));
    assert((reg.x + reg.z) <= (_width - 1));
    assert(reg.y < (_height - 1));
    assert((reg.y + reg.w) <= (_height - 1));
    charsize = sizeof(char);

    for(i = 0; i < reg.w; ++i)
    {
        if(_depth == 1 || _depth == 3)
            memcpy(_data + ((reg.y + i) * _width + reg.x) * charsize * _depth, data + (i * stride) * charsize,
                   reg.z * charsize * _depth);
        else   // depth 2 4
        {
            unsigned char bytes[4] = {0};   // 4 - alpha
            unsigned int  depth    = _depth - 1;

            for(int j = 0; j < reg.z; ++j)
            {
                unsigned int dst_shift = (((reg.y + i) * _width + reg.x + j) * charsize * _depth);
                unsigned int src_shift = (((i * stride) + j * depth) * charsize);

                bytes[0] = data[src_shift + 0];
                if(_depth == 4)
                {
                    bytes[1] = data[src_shift + 1];
                    bytes[2] = data[src_shift + 2];
                }
                bytes[3] = _depth == 1 ? bytes[0] : std::min(bytes[0] + bytes[1] + bytes[2], 255);

                _data[dst_shift + 0] = bytes[0];
                if(_depth == 4)
                {
                    _data[dst_shift + 1] = bytes[1];
                    _data[dst_shift + 2] = bytes[2];
                    _data[dst_shift + 3] = bytes[3];
                }
                else
                    _data[dst_shift + 1] = bytes[3];
            }
        }
    }
}

void AtlasTex::WriteAtlasToTGA(const std::string & name)
{
    WriteUncompressedTGA(name.c_str(), _depth, _width, _height, _data);
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
    assert(_data);

    glGenTextures(1, &atlTex);

    glBindTexture(GL_TEXTURE_2D, atlTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if(_depth == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
    }
    else if(_depth == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, _data);
    }
    else if(_depth == 2)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                     _data);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _width, _height, 0, GL_RED, GL_UNSIGNED_BYTE, _data);
    }
}

void AtlasTex::DeleteTexture()
{
    glDeleteTextures(1, &atlTex);
}
