#include "vertex_buffer.h"
#include <assert.h>
#include <algorithm>

VertexBuffer::VertexBuffer(ComponentsFlags format, uint32_t num_tex_channels)
    : m_tex_channels_count(num_tex_channels),
      m_components(format),
      m_state(State::NODATA)
{}

VertexBuffer::~VertexBuffer()
{
    clear();
}

void VertexBuffer::insertVertices(uint32_t const index, float const * pos,
                                  std::vector<float const *> const & tex, float const * norm,
                                  uint32_t const vcount)
{
    // This function inserts 'vcount' new vertices *before* the vertex currently at 'index'.
    // Assumes contiguous block layout:
    // - m_dynamic_buffer: PPP...NNN...
    // - m_static_bufffer: T0T0T0...T1T1T1...

    assert(index <= m_vertex_count);   // Allow insertion at the end (index == m_vertex_count)
    assert(pos);
    if(vcount == 0)
        return;

    // --- Handle m_dynamic_buffer (Positions and Normals) ---
    std::vector<float> new_dynamic_buffer;
    uint32_t const     new_total_vertices     = m_vertex_count + vcount;
    uint32_t const     pos_floats_per_vertex  = 3;   // Assuming 3 floats (x,y,z) for position
    uint32_t const     norm_floats_per_vertex = 3;   // Assuming 3 floats (x,y,z) for normal

    // Estimate capacity for new_dynamic_buffer
    uint32_t dynamic_capacity_estimation = new_total_vertices * pos_floats_per_vertex;
    if(m_components[ComponentsBitPos::normal])
    {
        dynamic_capacity_estimation += new_total_vertices * norm_floats_per_vertex;
    }
    new_dynamic_buffer.reserve(dynamic_capacity_estimation);

    // 1. Add positions
    // Copy existing positions before the insertion point 'index'
    new_dynamic_buffer.insert(new_dynamic_buffer.end(), m_dynamic_buffer.begin(),
                              m_dynamic_buffer.begin() + index * pos_floats_per_vertex);
    // Insert new position data
    new_dynamic_buffer.insert(new_dynamic_buffer.end(), pos, pos + vcount * pos_floats_per_vertex);
    // Copy existing positions after the insertion point 'index' (if any)
    if(index < m_vertex_count)
    {
        new_dynamic_buffer.insert(new_dynamic_buffer.end(),
                                  m_dynamic_buffer.begin() + index * pos_floats_per_vertex,
                                  m_dynamic_buffer.begin() + m_vertex_count * pos_floats_per_vertex);
    }

    // 2. Add normals (if this component is active)
    if(m_components[ComponentsBitPos::normal])
    {
        assert(norm);
        // Normals are stored after all position data.
        // Calculate the starting point of the normal block in the old m_dynamic_buffer.
        uint32_t old_pos_block_total_floats = m_vertex_count * pos_floats_per_vertex;

        // Copy existing normals before the insertion point 'index'
        new_dynamic_buffer.insert(
            new_dynamic_buffer.end(), m_dynamic_buffer.begin() + old_pos_block_total_floats,
            m_dynamic_buffer.begin() + old_pos_block_total_floats + index * norm_floats_per_vertex);
        // Insert new normal data
        new_dynamic_buffer.insert(new_dynamic_buffer.end(), norm, norm + vcount * norm_floats_per_vertex);
        // Copy existing normals after the insertion point 'index' (if any)
        if(index < m_vertex_count)
        {
            new_dynamic_buffer.insert(new_dynamic_buffer.end(),
                                      m_dynamic_buffer.begin() + old_pos_block_total_floats
                                          + index * norm_floats_per_vertex,
                                      m_dynamic_buffer.begin() + old_pos_block_total_floats
                                          + m_vertex_count * norm_floats_per_vertex);
        }
    }
    m_dynamic_buffer.swap(new_dynamic_buffer);

    // --- Handle m_static_bufffer (Texture Coordinates) ---
    if(m_components[ComponentsBitPos::tex])
    {
        assert(tex.size() == m_tex_channels_count);
        uint32_t const     tex_floats_per_vertex = 2;   // Assuming 2 floats (u,v) for texture coordinates
        std::vector<float> new_static_buffer;

        uint32_t static_capacity_estimation =
            new_total_vertices * tex_floats_per_vertex * m_tex_channels_count;
        new_static_buffer.reserve(static_capacity_estimation);

        for(uint32_t i = 0; i < m_tex_channels_count; ++i)
        {
            assert(tex[i]);
            // Calculate the start of the data block for the current texture channel 'i' in the old
            // m_static_bufffer. Each channel's data is contiguous: T0_data T1_data ... So, channel 'i' starts
            // after (i * m_vertex_count * tex_floats_per_vertex) floats.
            uint32_t old_tex_channel_block_start_offset = i * m_vertex_count * tex_floats_per_vertex;

            // Copy existing tex coords for this channel, before the insertion point 'index'
            new_static_buffer.insert(new_static_buffer.end(),
                                     m_static_bufffer.begin() + old_tex_channel_block_start_offset,
                                     m_static_bufffer.begin() + old_tex_channel_block_start_offset
                                         + index * tex_floats_per_vertex);
            // Insert new tex coords for this channel
            new_static_buffer.insert(new_static_buffer.end(), tex[i],
                                     tex[i] + vcount * tex_floats_per_vertex);
            // Copy existing tex coords for this channel, after the insertion point 'index' (if any)
            if(index < m_vertex_count)
            {
                new_static_buffer.insert(new_static_buffer.end(),
                                         m_static_bufffer.begin() + old_tex_channel_block_start_offset
                                             + index * tex_floats_per_vertex,
                                         m_static_bufffer.begin() + old_tex_channel_block_start_offset
                                             + m_vertex_count * tex_floats_per_vertex);
            }
        }
        m_static_bufffer.swap(new_static_buffer);
    }

    m_vertex_count += vcount;
    m_state         = State::INITDATA;
}

void VertexBuffer::insertIndices(uint32_t const index, uint32_t const * indices, uint32_t const icount)
{
    assert(index < m_indices.size());
    assert(indices);

    auto ind_it = m_indices.begin() + index;
    m_indices.insert(ind_it, indices, indices + icount);

    m_state = State::INITDATA;
}

void VertexBuffer::pushBack(float const * pos, std::vector<float const *> const & tex, float const * norm,
                            uint32_t const vcount, uint32_t const * indices, uint32_t const icount)
{
    assert(pos);
    if(icount > 0)
    {   // Ensure indices is valid if we are going to use it.
        assert(indices);
    }
    if(vcount == 0)
        return;   // Nothing to add if no vertices

    uint32_t const vstart = m_vertex_count;
    uint32_t const istart = static_cast<uint32_t>(m_indices.size());

    uint32_t const pos_floats_per_vertex  = 3;
    uint32_t const norm_floats_per_vertex = 3;
    uint32_t const tex_floats_per_vertex  = 2;

    // --- Handle m_dynamic_buffer (Positions and Normals) ---
    // Layout: P_block_old N_block_old
    // Target: P_block_old P_block_new N_block_old N_block_new
    auto pos_it_end = m_dynamic_buffer.begin() + m_vertex_count * pos_floats_per_vertex;

    m_dynamic_buffer.insert(pos_it_end, pos, pos + vcount * pos_floats_per_vertex);
    if(m_components[ComponentsBitPos::normal])
        m_dynamic_buffer.insert(m_dynamic_buffer.end(), norm, norm + vcount * norm_floats_per_vertex);

    // --- Handle m_static_bufffer (Texture Coordinates) ---
    // Layout: T0_block_old T1_block_old ... Tn-1_block_old
    // Target: T0_block_old T0_block_new T1_block_old T1_block_new ...
    if(m_components[ComponentsBitPos::tex])
    {
        assert(!tex.empty() && tex.size() == m_tex_channels_count);

        for(uint32_t i = 0; i < m_tex_channels_count; ++i)
        {
            assert(tex[i]);

            uint32_t offset_floats = (i * m_vertex_count * tex_floats_per_vertex)
                                     + (m_vertex_count * tex_floats_per_vertex)
                                     + (i * vcount * tex_floats_per_vertex);

            auto tex_insertion_iterator = m_static_bufffer.begin() + offset_floats;
            m_static_bufffer.insert(tex_insertion_iterator, tex[i], tex[i] + vcount * tex_floats_per_vertex);
        }
    }

    // --- Handle Indices ---
    if(icount > 0)
    {
        m_indices.insert(m_indices.end(), indices, indices + icount);
        for(uint32_t i = 0; i < icount; i++)
        {
            m_indices[istart + i] += vstart;
        }
    }

    m_vertex_count += vcount;
    m_state         = State::INITDATA;
}

void VertexBuffer::eraseVertices(uint32_t const first, uint32_t const last)
{
    assert(last > first);
    assert(last <= m_vertex_count);

    if(first >= m_vertex_count || first >= last)
    {   // If 'first' is out of bounds or range is invalid/empty.
        return;
    }

    uint32_t const count_to_erase         = last - first;
    uint32_t const pos_floats_per_vertex  = 3;
    uint32_t const norm_floats_per_vertex = 3;
    uint32_t const tex_floats_per_vertex  = 2;

    // --- Handle m_dynamic_buffer (Positions and Normals) ---
    // Layout: P_block N_block
    // Erase from P_block:
    auto p_erase_begin_it = m_dynamic_buffer.begin() + first * pos_floats_per_vertex;
    auto p_erase_end_it   = m_dynamic_buffer.begin() + last * pos_floats_per_vertex;
    m_dynamic_buffer.erase(p_erase_begin_it, p_erase_end_it);

    if(m_components[ComponentsBitPos::normal])
    {
        // Normals are after ALL positions.
        // Original start of N_block was at m_vertex_count * pos_floats_per_vertex (relative to old
        // dynamic_buffer start). After erasing 'count_to_erase' positions, the P_block is now (m_vertex_count
        // - count_to_erase) * pos_floats_per_vertex long. This is the new offset from dynamic_buffer.begin()
        // to where the N_block data starts.
        uint32_t n_block_new_start_offset = (m_vertex_count - count_to_erase) * pos_floats_per_vertex;

        auto n_erase_begin_it =
            m_dynamic_buffer.begin() + n_block_new_start_offset + first * norm_floats_per_vertex;
        auto n_erase_end_it =
            m_dynamic_buffer.begin() + n_block_new_start_offset + last * norm_floats_per_vertex;
        m_dynamic_buffer.erase(n_erase_begin_it, n_erase_end_it);
    }

    // --- Handle m_static_bufffer (Texture Coordinates) ---
    // Layout: T0_block T1_block ...
    if(m_components[ComponentsBitPos::tex])
    {
        uint32_t total_tex_floats_erased_from_previous_channels = 0;
        for(uint32_t i = 0; i < m_tex_channels_count; ++i)
        {
            // Calculate start of this channel's data in the original m_static_bufffer
            uint32_t channel_i_original_start_offset = i * m_vertex_count * tex_floats_per_vertex;

            auto t_erase_begin_it = m_static_bufffer.begin() + channel_i_original_start_offset
                                    + first * tex_floats_per_vertex
                                    - total_tex_floats_erased_from_previous_channels;
            auto t_erase_end_it = t_erase_begin_it + count_to_erase * tex_floats_per_vertex;

            m_static_bufffer.erase(t_erase_begin_it, t_erase_end_it);

            total_tex_floats_erased_from_previous_channels += count_to_erase * tex_floats_per_vertex;
        }
    }

    m_indices.erase(m_indices.begin() + first, m_indices.begin() + last);

    for(uint32_t i = 0; i < m_indices.size(); i++)
    {
        if(m_indices[i] > first)
        {
            m_indices[i] -= count_to_erase;
        }
    }

    m_state         = State::INITDATA;
    m_vertex_count -= count_to_erase;
}

void VertexBuffer::clear()
{
    m_state = State::NODATA;

    m_static_bufffer.resize(0);
    m_dynamic_buffer.resize(0);
    m_indices.resize(0);
    m_vertex_count = 0;
}

void VertexBuffer::updateDynamicBuffer(std::vector<glm::vec3> const & pos,
                                       std::vector<glm::vec3> const & norm)
{
    assert((pos.size() == m_vertex_count) && (norm.size() == m_vertex_count));
    assert(getComponentsFlags()[VertexBuffer::ComponentsBitPos::normal]);

    std::vector<float> new_dynamic_buffer(m_vertex_count * 3 * 2, 0.f);

    float const * start = &pos[0].x;
    float const * end   = start + m_vertex_count * 3;
    std::copy(start, end, new_dynamic_buffer.begin());

    start = &norm[0].x;
    end   = start + m_vertex_count * 3;
    std::copy(start, end, new_dynamic_buffer.begin() + m_vertex_count * 3);

    m_dynamic_buffer.swap(new_dynamic_buffer);
}

void Add2DRectangle(VertexBuffer & vb, float x0, float y0, float x1, float y1, float s0, float t0, float s1,
                    float t1)
{
    assert(vb.getNumTexChannels() == 1);
    // Add2DRectangle provides no normal data, so the VertexBuffer should not expect it.
    assert(!vb.getComponentsFlags()[VertexBuffer::ComponentsBitPos::normal]);

    uint32_t indices[6] = {0, 1, 2, 0, 3, 1};

    float vertices[4 * 3]  = {x0, y0, 0.f, x1, y1, 0.f, x0, y1, 0.f, x1, y0, 0.f};
    float tex_coord[4 * 2] = {s0, t0, s1, t1, s0, t1, s1, t0};

    vb.pushBack(vertices, {tex_coord}, nullptr, 4, indices, 6);
}
