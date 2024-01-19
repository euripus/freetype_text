#include "gl15_vb.h"
#include <GL/glew.h>
#include <assert.h>

GL15VertexBuffer::GL15VertexBuffer(ComponentsFlags format)
    : m_pos_id(0),
      m_tex_id(0),
      m_norm_id(0),
      m_indices_id(0),
      m_components(format),
      m_is_generated(false),
      m_state(State::NOINIT)
{}

GL15VertexBuffer::~GL15VertexBuffer()
{
    clear();

    glDeleteBuffers(1, &m_pos_id);
    if(m_components[ComponentsBitPos::tex])
        glDeleteBuffers(1, &m_tex_id);
    if(m_components[ComponentsBitPos::normal])
        glDeleteBuffers(1, &m_norm_id);
    glDeleteBuffers(1, &m_indices_id);
}

void GL15VertexBuffer::insertVertices(size_t const index, float const * pos, float const * tex,
                                      float const * norm, size_t const vcount)
{
    assert(index * 3 < m_pos.size());
    assert(pos);

    auto flt_it = m_pos.begin() + index * 3;
    m_pos.insert(flt_it, pos, pos + vcount * 3);
    if(m_components[ComponentsBitPos::tex])
    {
        assert(index * 2 < m_tex.size());
        assert(tex);

        auto tex_it = m_tex.begin() + index * 2;
        m_tex.insert(tex_it, tex, tex + vcount * 2);
    }
    if(m_components[ComponentsBitPos::normal])
    {
        assert(index * 3 < m_norm.size());
        assert(norm);

        auto norm_it = m_pos.begin() + index * 3;
        m_norm.insert(norm_it, norm, norm + vcount * 3);
    }

    m_state = State::INITDATA;
}

void GL15VertexBuffer::insertIndices(size_t const index, unsigned int const * indices, size_t const icount)
{
    assert(index < m_indices.size());
    assert(indices);

    auto ind_it = m_indices.begin() + index;
    m_indices.insert(ind_it, indices, indices + icount);

    m_state = State::INITDATA;
}

void GL15VertexBuffer::pushBack(float const * pos, float const * tex, float const * norm, size_t const vcount,
                                unsigned int const * indices, size_t const icount)
{
    assert(pos);
    assert(indices);

    unsigned int vstart = m_pos.size() / 3;
    unsigned int istart = m_indices.size();

    m_pos.insert(m_pos.end(), pos, pos + vcount * 3);
    if(m_components[ComponentsBitPos::tex])
        m_tex.insert(m_tex.end(), tex, tex + vcount * 2);
    if(m_components[ComponentsBitPos::normal])
        m_norm.insert(m_norm.end(), norm, norm + vcount * 3);
    m_indices.insert(m_indices.end(), indices, indices + icount);

    for(unsigned int i = 0; i < icount; i++)
    {
        m_indices[istart + i] += vstart;
    }

    m_state = State::INITDATA;
}

void GL15VertexBuffer::eraseVertices(size_t const first, size_t const last)
{
    assert(last > first);
    assert(last * 3 < m_pos.size());
    assert(last < m_indices.size());

    m_pos.erase(m_pos.begin() + first * 3, m_pos.begin() + last * 3);
    if(m_components[ComponentsBitPos::tex])
        m_tex.erase(m_tex.begin() + first * 2, m_tex.begin() + last * 2);
    if(m_components[ComponentsBitPos::normal])
        m_norm.erase(m_norm.begin() + first * 3, m_norm.begin() + last * 3);
    m_indices.erase(m_indices.begin() + first, m_indices.begin() + last);

    for(unsigned int i = 0; i < m_indices.size(); i++)
    {
        if(m_indices[i] > first)
        {
            m_indices[i] -= (last - first);
        }
    }

    m_state = State::INITDATA;
}

void GL15VertexBuffer::clear()
{
    m_state = State::NOINIT;
    if(m_is_generated)
    {
        glBindBuffer(GL_ARRAY_BUFFER_ARB, m_pos_id);
        glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
        if(m_components[ComponentsBitPos::tex])
        {
            glBindBuffer(GL_ARRAY_BUFFER_ARB, m_tex_id);
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
        }
        if(m_components[ComponentsBitPos::normal])
        {
            glBindBuffer(GL_ARRAY_BUFFER_ARB, m_norm_id);
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
    }

    m_pos.clear();
    m_tex.clear();
    m_norm.clear();
    m_indices.clear();
}

void GL15VertexBuffer::upload()
{
    assert(m_state == State::INITDATA);

    if(!m_is_generated)
        glGenBuffers(1, &m_pos_id);
    glBindBuffer(GL_ARRAY_BUFFER_ARB, m_pos_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_pos.size(), &m_pos[0], GL_STATIC_DRAW);
    if(m_components[ComponentsBitPos::tex])
    {
        if(!m_is_generated)
            glGenBuffers(1, &m_tex_id);
        glBindBuffer(GL_ARRAY_BUFFER_ARB, m_tex_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_tex.size(), &m_tex[0], GL_STATIC_DRAW);
    }
    if(m_components[ComponentsBitPos::normal])
    {
        if(!m_is_generated)
            glGenBuffers(1, &m_norm_id);
        glBindBuffer(GL_ARRAY_BUFFER_ARB, m_norm_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_norm.size(), &m_norm[0], GL_STATIC_DRAW);
    }

    if(!m_is_generated)
    {
        glGenBuffers(1, &m_indices_id);
        m_is_generated = true;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices.size(), &m_indices[0],
                 GL_STATIC_DRAW);

    m_state = State::UPLOAD;
}

void GL15VertexBuffer::drawBuffer()
{
    if(m_state == State::UPLOAD)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_pos_id);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, static_cast<void *>(nullptr));

        if(m_components[ComponentsBitPos::tex])
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_tex_id);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, 0, static_cast<void *>(nullptr));
        }

        if(m_components[ComponentsBitPos::normal])
        {
            glBindBuffer(GL_ARRAY_BUFFER, m_norm_id);
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, 0, static_cast<void *>(nullptr));
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_id);

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, static_cast<char *>(nullptr));

        glDisableClientState(GL_VERTEX_ARRAY);
        if(m_components[ComponentsBitPos::tex])
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(m_components[ComponentsBitPos::normal])
            glDisableClientState(GL_NORMAL_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    }
}
