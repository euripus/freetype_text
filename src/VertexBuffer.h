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

    GLenum        m_mode;
    State         m_state;
    std::uint32_t m_num_vert_comp;
    bool          m_is_generated;

    std::vector<VertexAttrib> m_attributes;

public:
    VertexBuffer(char const * format);
    virtual ~VertexBuffer();

    // unsigned int NumTris() const { return _indices.size()/3; }
    unsigned int GetNumVertComponents() const { return m_num_vert_comp; }

    void VertexBufferInsertVertices(size_t const index, float const * vertices, size_t const vcount);
    void VertexBufferInsertIndices(size_t const index, unsigned int const * indices, size_t const icount);
    void VertexBufferPushBack(float const * vertices, size_t const vcount, unsigned int const * indices,
                              size_t const icount);

    void EraseVertices(size_t const first, size_t const last);
    void Clear();

    void InitAttribLocation();

    void VertexBufferUpload();
    void DeleteGPUBuffers();

    void DrawBuffer();
};

void add2DRectangle(VertexBuffer & vb, float x0, float y0, float x1, float y1, float s0, float t0, float s1,
                    float t1);

#endif   // VERTEXBUFFER_H
