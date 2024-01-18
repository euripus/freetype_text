#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <vector>
#include <glm/glm.hpp>

class VertexBuffer
{
public:
    enum class State
    {
        NOINIT,
        INITDATA,
        UPLOAD
    };

    struct ComponentsBitPos
    {
        static int const pos    = 0;
        static int const tex    = 1;
        static int const normal = 2;
    };

    using ComponentsFlags = std::bitset<3>;   // pos always true

    constexpr ComponentsFlags pos          = 0b000001;   // pos
    constexpr ComponentsFlags pos_tex      = 0b000011;   // pos + tex
    constexpr ComponentsFlags pos_tex_norm = 0b000111;   // pos + tex + norm

    void insertVertices(size_t const index, float const * vertices, size_t const vcount);
    void insertIndices(size_t const index, unsigned int const * indices, size_t const icount);
    void pushBack(float const * vertices, size_t const vcount, unsigned int const * indices,
                  size_t const icount);

    void eraseVertices(size_t const first, size_t const last);
    void clear();

    void upload();
    void deleteGPUBuffers();

    void drawBuffer();

    ComponentsFlags getComponentsFlags() const { return m_components; }

private:
    std::vector<float> m_pos;
    std::vector<float> m_tex;
    std::vector<float> m_norm;
    GLuint             m_pos_id;
    GLuint             m_tex_id;
    GLuint             m_norm_id;

    std::vector<unsigned int> m_indices;
    GLuint                    m_indices_id;

    ComponentsFlags m_components;
    State           m_state;
};

void VertexBuffer::drawBuffer()
{
    if(m_state == VertexBuffer::State::VB_UPLOAD)
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

#endif
