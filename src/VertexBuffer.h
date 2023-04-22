#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "VertexAttrib.h"
#include <GL/glew.h>
#include <vector>

class VertexBuffer
{
public:
    enum class State
    {
        VB_NOINIT,
        VB_INITDATA,
        VB_UPLOAD
    };

private:
    std::vector<float> m_vertices;
    GLuint             m_vertices_id;

    std::vector<unsigned int> m_indices;
    GLuint                    m_indices_id;

    GLenum       m_mode;
    State        m_state;
    unsigned int m_num_vert_comp;
    bool         m_is_generated;

    std::vector<VertexAttrib> m_attributes;

public:
    VertexBuffer(char const * format);
    virtual ~VertexBuffer();

    // unsigned int NumTris() const { return _indices.size()/3; }
    unsigned int GetNumVertComponents() const { return m_num_vert_comp; }

    void VertexBufferInsertVertices(const size_t index, float const * vertices, const size_t vcount);
    void VertexBufferInsertIndices(const size_t index, unsigned int const * indices, const size_t icount);
    void VertexBufferPushBack(float const * vertices, const size_t vcount, unsigned int const * indices,
                              const size_t icount);

    void EraseVertices(const size_t first, const size_t last);
    void Clear();

    void InitAttribLocation();

    void VertexBufferUpload();
    void DeleteGPUBuffers();

    void DrawBuffer();
};

#endif   // VERTEXBUFFER_H
