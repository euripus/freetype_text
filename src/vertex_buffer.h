#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <vector>
#include <bitset>

class GL15VertexBuffer
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
        constexpr static int pos    = 0;
        constexpr static int tex    = 1;
        constexpr static int normal = 2;
    };

    using ComponentsFlags = std::bitset<8>;   // pos always true

    constexpr static ComponentsFlags pos          = 0b000001;   // pos
    constexpr static ComponentsFlags pos_tex      = 0b000011;   // pos + tex
    constexpr static ComponentsFlags pos_tex_norm = 0b000111;   // pos + tex + norm

    GL15VertexBuffer(ComponentsFlags format);
    virtual ~GL15VertexBuffer();

    void insertVertices(size_t const index, float const * pos, float const * tex, float const * norm,
                        size_t const vcount);
    void insertIndices(size_t const index, unsigned int const * indices, size_t const icount);
    void pushBack(float const * pos, float const * tex, float const * norm, size_t const vcount,
                  unsigned int const * indices, size_t const icount);

    void eraseVertices(size_t const first, size_t const last);
    void clear();

    void upload();

    void drawBuffer();

    ComponentsFlags getComponentsFlags() const { return m_components; }

private:
    std::vector<float> m_pos;
    std::vector<float> m_tex;
    std::vector<float> m_norm;
    uint32_t           m_pos_id;
    uint32_t           m_tex_id;
    uint32_t           m_norm_id;

    std::vector<unsigned int> m_indices;
    uint32_t                  m_indices_id;

    ComponentsFlags const m_components;
    bool                  m_is_generated;
    State                 m_state;
};

void add2DRectangle(VertexBuffer & vb, float x0, float y0, float x1, float y1, float s0, float t0, float s1,
                    float t1);

#endif
