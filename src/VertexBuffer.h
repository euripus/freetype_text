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
    std::vector<float> _vertices;
    GLuint             vertices_id;

    std::vector<unsigned int> _indices;
    GLuint                    indices_id;

    GLenum       _mode;
    State        _state;
    unsigned int _numVertComp;
    bool         _isGenerated;

    std::vector<VertexAttrib> _attributes;

public:
    VertexBuffer(const char * format);
    virtual ~VertexBuffer();

    // unsigned int NumTris() const { return _indices.size()/3; }
    unsigned int GetNumVertComponents() const { return _numVertComp; }

    void VertexBufferInsertVertices(const size_t index, const float * vertices, const size_t vcount);
    void VertexBufferInsertIndices(const size_t index, const unsigned int * indices, const size_t icount);
    void VertexBufferPushBack(const float * vertices, const size_t vcount, const unsigned int * indices,
                              const size_t icount);

    void EraseVertices(const size_t first, const size_t last);
    void Clear();

    void InitAttribLocation();

    void VertexBufferUpload();
    void DeleteGPUBuffers();

    void DrawBuffer();
};

#endif   // VERTEXBUFFER_H
