#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include <vector>
#include <bitset>
#include <cstdint>
#include <glm/glm.hpp>

class VertexBuffer
{
public:
    enum class State
    {
        NODATA,
        INITDATA,
        COMMITTED
    };

    struct ComponentsBitPos
    {
        constexpr static int pos    = 0;
        constexpr static int normal = 1;
        constexpr static int tex    = 2;
    };

    using ComponentsFlags = std::bitset<8>;   // pos always true

    constexpr static ComponentsFlags null         = 0b000000;   // null
    constexpr static ComponentsFlags pos          = 0b000001;   // pos
    constexpr static ComponentsFlags pos_norm     = 0b000011;   // pos + norm
    constexpr static ComponentsFlags pos_norm_tex = 0b000111;   // pos + norm + tex

    VertexBuffer(ComponentsFlags format = pos_norm_tex, uint32_t num_tex_channels = 1);
    ~VertexBuffer();

    void insertVertices(uint32_t const index, float const * pos, std::vector<float const *> const & tex,
                        float const * norm, uint32_t const vcount);
    void insertIndices(uint32_t const index, uint32_t const * indices, uint32_t const icount);
    void pushBack(float const * pos, std::vector<float const *> const & tex, float const * norm,
                  uint32_t const vcount, uint32_t const * indices, uint32_t const icount);

    void eraseVertices(uint32_t const first, uint32_t const last);
    void clear();

    ComponentsFlags getComponentsFlags() const { return m_components; }
    uint32_t        getNumTexChannels() const { return m_tex_channels_count; }
    uint32_t        getNumVertex() const { return m_vertex_count; }
    uint32_t        getNumTriangles() const { return static_cast<uint32_t>(m_indices.size()) / 3; }
    void updateDynamicBuffer(std::vector<glm::vec3> const & pos, std::vector<glm::vec3> const & norm);

private:
    std::vector<float> m_static_bufffer;   // for tex0 tex1 ...
    std::vector<float> m_dynamic_buffer;   // for pos norm
    uint32_t           m_vertex_count       = 0;
    uint32_t           m_tex_channels_count = 0;
    uint32_t           m_static_bufffer_id  = 0;
    uint32_t           m_dynamic_buffer_id  = 0;

    std::vector<uint32_t> m_indices;
    uint32_t              m_indices_id = 0;

    ComponentsFlags const m_components;
    bool                  m_is_generated = false;
    State                 m_state        = State::NODATA;

    friend class RendererBase;
};

void Add2DRectangle(VertexBuffer & vb, float x0, float y0, float x1, float y1, float s0, float t0, float s1,
                    float t1);

#endif
