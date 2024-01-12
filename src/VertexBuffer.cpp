#include "VertexBuffer.h"
#include "VertexAttrib.h"
#include <GL/glew.h>
#include <algorithm>
#include <cassert>
#include <cstring>

char * strndup(char const * s, size_t n)
{
    char * result;
    size_t len = std::strlen(s);

    if(n < len)
        len = n;

    result = new char[len + 1];
    if(!result)
        return 0;

    result[len] = '\0';
    return static_cast<char *>(std::memcpy(result, s, len));
}

char * strdup(char const * s)
{
    char * result;
    size_t len = std::strlen(s);

    result = new char[len + 1];
    if(!result)
        return 0;

    result[len] = '\0';
    return static_cast<char *>(std::memcpy(result, s, len));
}

VertexBuffer::VertexBuffer(char const * format)
    : m_vertices_id(0),
      m_indices_id(0),
      m_mode(0),
      m_state(VertexBuffer::State::VB_NOINIT),
      m_num_vert_comp(0),
      m_is_generated(false)
{
    assert(format);

    char const *start = 0, *end = 0;

    start = format;
    do
    {
        VertexAttrib new_attr;
        char *       desc = 0;
        end               = const_cast<char *>(std::strchr(start + 1, ','));

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

        new_attr.pointer  = sizeof(float) * m_num_vert_comp;
        m_num_vert_comp  += new_attr.size;
        m_attributes.push_back(new_attr);
    } while(end);

    std::for_each(m_attributes.begin(), m_attributes.end(),
                  [=](VertexAttrib & attr) { attr.stride = sizeof(float) * m_num_vert_comp; });
}

VertexBuffer::~VertexBuffer()
{
    Clear();
    DeleteGPUBuffers();
    m_attributes.clear();
}

void VertexBuffer::Clear()
{
    m_state = VertexBuffer::State::VB_NOINIT;
    if(m_is_generated)
    {
        glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vertices_id);
        glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
    }

    m_vertices.clear();
    m_indices.clear();
}

void VertexBuffer::VertexBufferInsertVertices(size_t const index, float const * vertices, size_t const vcount)
{
    assert(index * m_num_vert_comp < m_vertices.size());
    assert(vertices);

    auto flt_it = m_vertices.begin() + index * m_num_vert_comp;
    m_vertices.insert(flt_it, vertices, vertices + vcount);
}

void VertexBuffer::VertexBufferInsertIndices(size_t const index, unsigned int const * indices,
                                             size_t const icount)
{
    assert(index < m_indices.size());
    assert(indices);

    auto ind_it = m_indices.begin() + index;
    m_indices.insert(ind_it, indices, indices + icount);
}

void VertexBuffer::VertexBufferPushBack(float const * vertices, size_t const vcount,
                                        unsigned int const * indices, size_t const icount)
{
    assert(vertices);
    assert(indices);
    assert(m_num_vert_comp != 0);

    unsigned int vstart = m_vertices.size() / m_num_vert_comp;
    unsigned int istart = m_indices.size();

    m_vertices.insert(m_vertices.end(), vertices, vertices + vcount * m_num_vert_comp);
    m_indices.insert(m_indices.end(), indices, indices + icount);

    for(unsigned int i = 0; i < icount; i++)
    {
        m_indices[istart + i] += vstart;
    }

    m_state = VertexBuffer::State::VB_INITDATA;
}

void VertexBuffer::EraseVertices(size_t const first, size_t const last)
{
    assert(last * m_num_vert_comp < m_vertices.size());
    assert(last < m_indices.size());

    m_vertices.erase(m_vertices.begin() + first * m_num_vert_comp,
                     m_vertices.begin() + last * m_num_vert_comp);
    m_indices.erase(m_indices.begin() + first, m_indices.begin() + last);

    for(unsigned int i = 0; i < m_indices.size(); i++)
    {
        if(m_indices[i] > first)
        {
            m_indices[i] -= (last - first);
        }
    }
}

void VertexBuffer::VertexBufferUpload()
{
    assert(m_state == VertexBuffer::State::VB_INITDATA);

    if(!m_is_generated)
        glGenBuffers(1, &m_vertices_id);
    glBindBuffer(GL_ARRAY_BUFFER_ARB, m_vertices_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);

    if(!m_is_generated)
    {
        glGenBuffers(1, &m_indices_id);
        m_is_generated = true;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices.size(), &m_indices[0],
                 GL_STATIC_DRAW);

    m_state = VertexBuffer::State::VB_UPLOAD;
}

void VertexBuffer::DeleteGPUBuffers()
{
    glDeleteBuffers(1, &m_vertices_id);
    glDeleteBuffers(1, &m_indices_id);

    m_state        = VertexBuffer::State::VB_INITDATA;
    m_is_generated = false;
}

void VertexBuffer::DrawBuffer()
{
    if(m_state == VertexBuffer::State::VB_UPLOAD)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertices_id);
        std::for_each(m_attributes.begin(), m_attributes.end(),
                      [](VertexAttrib & attr) { attr.VertexAttribEnable(); });
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, static_cast<char *>(nullptr));

        std::for_each(m_attributes.begin(), m_attributes.end(),
                      [](VertexAttrib & attr) { attr.VertexAttribDisable(); });
        glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}

void VertexBuffer::InitAttribLocation()
{
    /*
     *        Explicit attribute location
     * Pos  - position data - 0
     * Norm - normal vector - 1
     * Tex  - texture coord - 2
     */

    std::for_each(m_attributes.begin(), m_attributes.end(), [](VertexAttrib & attr) {
        if(attr.name == std::string("Pos"))
            attr.location = 0;
        else if(attr.name == std::string("Norm"))
            attr.location = 1;
        else if(attr.name == std::string("Tex"))
            attr.location = 2;
    });
}

void add2DRectangle(VertexBuffer & vb, float x0, float y0, float x1, float y1, float s0, float t0, float s1,
                    float t1)
{
    unsigned int indices[6] = {0, 1, 2, 0, 3, 1};

    float vertices[4 * 5] = {x0, y0, 0.0, s0, t0, x1, y1, 0.0, s1, t1,
                             x0, y1, 0.0, s0, t1, x1, y0, 0.0, s1, t0};

    vb.VertexBufferPushBack(vertices, 4, indices, 6);
}
