#include "VertexBuffer.h"
#include "VertexAttrib.h"
#include <GL/glew.h>
#include <algorithm>
#include <cassert>
#include <cstring>

char * strndup(const char * s, size_t n)
{
    char * result;
    size_t len = std::strlen(s);

    if(n < len)
        len = n;

    result = new char[len + 1];
    if(!result)
        return 0;

    result[len] = '\0';
    return (char *)std::memcpy(result, s, len);
}

char * strdup(const char * s)
{
    char * result;
    size_t len = std::strlen(s);

    result = new char[len + 1];
    if(!result)
        return 0;

    result[len] = '\0';
    return (char *)std::memcpy(result, s, len);
}

VertexBuffer::VertexBuffer(const char * format) :
    vertices_id(0),
    indices_id(0),
    _mode(0),
    _isGenerated(false),
    _state(VertexBuffer::State::VB_NOINIT),
    _numVertComp(0)
{
    assert(format);

    const char *start = 0, *end = 0;

    start = format;
    do
    {
        VertexAttrib new_attr;
        char *       desc = 0;
        end               = (char *)(std::strchr(start + 1, ','));

        if(end == NULL)
        {
            desc = strdup(start);
        }
        else
        {
            desc = strndup(start, end - start);
        }
        new_attr = VertexAttrib::VertexAttribParse(desc);
        start    = end + 1;
        delete[] desc;

        new_attr.pointer = sizeof(float) * _numVertComp;
        _numVertComp += new_attr.size;
        _attributes.push_back(new_attr);
    } while(end);

    std::for_each(_attributes.begin(), _attributes.end(),
                  [=](VertexAttrib & attr) { attr.stride = sizeof(float) * _numVertComp; });
}

VertexBuffer::~VertexBuffer()
{
    Clear();
    DeleteGPUBuffers();
    _attributes.clear();
}

void VertexBuffer::Clear()
{
    _state = VertexBuffer::State::VB_NOINIT;
    if(_isGenerated)
    {
        glBindBuffer(GL_ARRAY_BUFFER_ARB, vertices_id);
        glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
    }

    _vertices.clear();
    _indices.clear();
}

void VertexBuffer::VertexBufferInsertVertices(const size_t index, const float * vertices, const size_t vcount)
{
    assert(index * _numVertComp < _vertices.size());
    assert(vertices);

    auto flt_it = _vertices.begin() + index * _numVertComp;
    _vertices.insert(flt_it, vertices, vertices + vcount);
}

void VertexBuffer::VertexBufferInsertIndices(const size_t index, const unsigned int * indices,
                                             const size_t icount)
{
    assert(index < _indices.size());
    assert(indices);

    auto ind_it = _indices.begin() + index;
    _indices.insert(ind_it, indices, indices + icount);
}

void VertexBuffer::VertexBufferPushBack(const float * vertices, const size_t vcount,
                                        const unsigned int * indices, const size_t icount)
{
    assert(vertices);
    assert(indices);
    assert(_numVertComp != 0);

    unsigned int vstart = _vertices.size() / _numVertComp;
    unsigned int istart = _indices.size();

    _vertices.insert(_vertices.end(), vertices, vertices + vcount * _numVertComp);
    _indices.insert(_indices.end(), indices, indices + icount);

    for(unsigned int i = 0; i < icount; i++)
    {
        _indices[istart + i] += vstart;
    }

    _state = VertexBuffer::State::VB_INITDATA;
}

void VertexBuffer::EraseVertices(const size_t first, const size_t last)
{
    assert(last * _numVertComp < _vertices.size());
    assert(last < _indices.size());

    _vertices.erase(_vertices.begin() + first * _numVertComp, _vertices.begin() + last * _numVertComp);
    _indices.erase(_indices.begin() + first, _indices.begin() + last);

    for(unsigned int i = 0; i < _indices.size(); i++)
    {
        if(_indices[i] > first)
        {
            _indices[i] -= (last - first);
        }
    }
}

void VertexBuffer::VertexBufferUpload()
{
    assert(_state == VertexBuffer::State::VB_INITDATA);

    if(!_isGenerated)
        glGenBuffers(1, &vertices_id);
    glBindBuffer(GL_ARRAY_BUFFER_ARB, vertices_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * _vertices.size(), &_vertices[0], GL_STATIC_DRAW);

    if(!_isGenerated)
    {
        glGenBuffers(1, &indices_id);
        _isGenerated = true;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * _indices.size(), &_indices[0],
                 GL_STATIC_DRAW);

    _state = VertexBuffer::State::VB_UPLOAD;
}

void VertexBuffer::DeleteGPUBuffers()
{
    glDeleteBuffers(1, &vertices_id);
    glDeleteBuffers(1, &indices_id);

    _state       = VertexBuffer::State::VB_INITDATA;
    _isGenerated = false;
}

void VertexBuffer::DrawBuffer()
{
    assert(_state == VertexBuffer::State::VB_UPLOAD);

    glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
    std::for_each(_attributes.begin(), _attributes.end(),
                  [](VertexAttrib & attr) { attr.VertexAttribEnable(); });
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, (char *)NULL);

    std::for_each(_attributes.begin(), _attributes.end(),
                  [](VertexAttrib & attr) { attr.VertexAttribDisable(); });
    glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void VertexBuffer::InitAttribLocation()
{
    /*
     *        Explicit attribute location
     * Pos  - position data - 0
     * Norm - normal vector - 1
     * Tex  - texture coord - 2
     */

    std::for_each(_attributes.begin(), _attributes.end(), [](VertexAttrib & attr) {
        if(attr.name == std::string("Pos"))
            attr.location = 0;
        else if(attr.name == std::string("Norm"))
            attr.location = 1;
        else if(attr.name == std::string("Tex"))
            attr.location = 2;
    });
}
