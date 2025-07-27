#ifndef RENDERER_H
#define RENDERER_H

#include "AABB.h"
#include "render_states.h"
#include "vertex_buffer.h"
#include "texture.h"
#include "../res/imagedata.h"

// simple openGL 1.5 renderer

class RendererBase
{
public:
    enum class MatrixType
    {
        PROJECTION,
        MODELVIEW
    };

    std::string const & getRenderVendor() const { return m_vendor; }
    std::string const & getRenderRenderer() const { return m_renderer; }
    std::string const & getRenderVersion() const { return m_version; }

    bool init();
    void terminate();
    bool checkExtensions() const;
    bool isInit() const { return m_initialized; }

    void setMatrix(MatrixType type, glm::mat4 const & matrix) const;
    void setIdentityMatrix(MatrixType type) const;

    // Vertex buffer functions
    void uploadBuffer(VertexBuffer & geo) const;
    void unloadBuffer(VertexBuffer const & geo) const;
    void deleteBuffer(VertexBuffer & geo) const;
    void bindVertexBuffer(VertexBuffer const * geo) const;   // must be called after bindSlots()
    void unbindVertexBuffer() const;                         // must be called before clearSlots()
    void draw(VertexBuffer const & geo) const;
    void drawIndexed(uint32_t first_index, uint32_t num_indices, uint32_t first_vert,
                     uint32_t num_verts) const;

    // Textures
    void          createTexture(Texture & tex) const;
    void          uploadTextureData(Texture & tex, tex::ImageData const & tex_data,
                                    Texture::CubeFace face = Texture::CubeFace::POS_X) const;
    void          destroyTexture(Texture & tex) const;
    bool          get2DTextureData(Texture const & tex, tex::ImageData & tex_data,
                                   Texture::CubeFace face = Texture::CubeFace::POS_X) const;
    void          applySamplerState(Texture const & tex) const;
    void          applyCombineStage(CombineStage const & combine) const;
    uint32_t      addTextureSlot(TextureSlot slot);
    TextureSlot & getTextureSlot(uint32_t slot_num);
    void          bindSlots() const;
    void          unbindSlots() const;
    void          clearSlots();
    void          unbindAndClearSlots();
    void          enableTextureCoordGeneration(std::uint32_t slot_num, uint32_t target) const;
    void          disableTextureCoordGeneration(std::uint32_t slot_num, uint32_t target) const;

    // Light`s
    void     clearLights() { m_lights_queue.resize(0); }
    void     clearLight(uint32_t index);
    uint32_t addLight(Light light);
    void     bindLights() const;
    void     unbindLights() const;

    // Frame buffer
    bool bindTextureAsFrameBuffer(Texture * color_tex, Texture * depth_tex = nullptr);
    void unbindTexturesFromFrameBuffer() const;
    void bindDefaultFbo();

    void enableClipPlane(uint32_t plane_num, glm::vec4 const & plane) const;
    void disableClipPlane(uint32_t plane_num) const;

    void setDrawColor(glm::vec4 const & color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)) const;

    // debug draw
    void drawBBox(AABB const & bbox, glm::mat4 const & object2world, glm::vec3 const & color);

    // Access to the current clearing parameters for the color, depth, and
    // stencil buffers.
    void              setClearColor(glm::vec4 const & clear_color) { m_clear_color = clear_color; }
    glm::vec4 const & getClearColor() const { return m_clear_color; }
    void              setClearDepth(float clear_depth) { m_clear_depth = clear_depth; }
    float             getClearDepth() const { return m_clear_depth; }
    void              setClearStencil(int32_t clear_stencil) { m_clear_stencil = clear_stencil; }
    int32_t           getClearStencil() const { return m_clear_stencil; }

    // Support for clearing the color, depth, and stencil buffers.
    void clearColorBuffer() const;
    void clearDepthBuffer() const;
    void clearStencilBuffer() const;
    void clearBuffers() const;

    void setViewport(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height);

    AlphaState   setAlphaState(AlphaState const & new_state);
    CullState    setCullState(CullState const & new_state);
    DepthState   setDepthState(DepthState const & new_state);
    OffsetState  setOffsetState(OffsetState const & new_state);
    StencilState setStencilState(StencilState const & new_state);
    WireState    setWireState(WireState const & new_state);

    AlphaState   getAlphaState() const { return m_alpha; }
    CullState    getCullState() const { return m_cull; }
    DepthState   getDepthState() const { return m_depth; }
    OffsetState  getOffsetState() const { return m_offset; }
    StencilState getStencilState() const { return m_stencil; }
    WireState    getWireState() const { return m_wire; }

private:
    void commitAlphaState() const;
    void commitCullState() const;
    void commitDepthState() const;
    void commitOffsetState() const;
    void commitStencilState() const;
    void commitWireState() const;
    void commitAllStates() const;

    bool m_initialized = false;

    glm::ivec2 m_viewport_pos  = {0, 0};
    glm::ivec2 m_viewport_size = {0, 0};

    std::string m_vendor   = {};
    std::string m_renderer = {};
    std::string m_version  = {};

    // states
    AlphaState   m_alpha;
    CullState    m_cull;
    DepthState   m_depth;
    OffsetState  m_offset;
    StencilState m_stencil;
    WireState    m_wire;

    glm::vec4 m_clear_color   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float     m_clear_depth   = 1.0f;
    int32_t   m_clear_stencil = 0;

    std::vector<Light> m_lights_queue;
    uint32_t           m_max_lights = 0;

    // bbox vbo
    uint32_t m_bbox_vbo_vertices = 0;
    uint32_t m_bbox_ibo_elements = 0;

    // default texture
    uint32_t m_default_texture = 0;

    // Texturing slots
    uint32_t                 m_max_texture_slots = 0;
    std::vector<TextureSlot> m_texture_slots;

    // FBO
    uint32_t m_default_fbo      = 0;
    uint32_t m_custom_fbo       = 0;
    uint32_t m_custom_fbo_depth = 0;

    uint32_t m_max_clip_planes = 0;

    // mutables
    mutable VertexBuffer::ComponentsFlags m_last_binded_vbo_components = {};
    mutable bool                          m_fbo_color_attached         = false;
};

#endif
