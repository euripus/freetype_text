#include "renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <array>
#include <stdlib.h>
#include <stdexcept>

// Mappings
static std::array<GLenum, static_cast<uint32_t>(CompareMode::QUANTITY)> const g_gl_compare_mode = {
    {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS}
};

static std::array<GLenum, static_cast<uint32_t>(AlphaState::SrcBlendMode::QUANTITY)> const
    g_gl_alpha_src_blend = {
        {GL_ZERO, GL_ONE, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
         GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA_SATURATE, GL_CONSTANT_COLOR,
         GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA}
};

static std::array<GLenum, static_cast<uint32_t>(AlphaState::DstBlendMode::QUANTITY)> const
    g_gl_alpha_dst_blend = {
        {GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
         GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
         GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA}
};

static std::array<GLenum, static_cast<uint32_t>(StencilState::OperationType::QUANTITY)> const
    g_gl_stencil_operation = {
        {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT}
};

static std::array<GLint, static_cast<uint32_t>(ImageState::Filter::QUANTITY)> const g_gl_tex_filter = {
    {GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR,
     GL_LINEAR_MIPMAP_LINEAR}
};

static std::array<GLint, static_cast<uint32_t>(ImageState::Wrap::QUANTITY)> const g_gl_tex_wrap = {
    {GL_CLAMP, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT}
};

static std::array<uint32_t, static_cast<uint32_t>(ImageState::Type::QUANTITY)> const g_texture_gl_types{
    {0, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP}
};

static std::array<uint32_t, static_cast<uint32_t>(CombineStage::CombineMode::QUANTITY)> const
    g_texture_gl_combine_modes{
        {GL_ADD, GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE, GL_COMBINE}
};

static std::array<uint32_t, static_cast<uint32_t>(CombineStage::CombineFunctions::QUANTITY)> const
    g_texture_gl_combine_functions{
        {GL_REPLACE, GL_MODULATE, GL_ADD, GL_ADD_SIGNED, GL_INTERPOLATE, GL_SUBTRACT, GL_DOT3_RGB,
         GL_DOT3_RGBA}
};

static std::array<uint32_t, static_cast<uint32_t>(CombineStage::SrcType::QUANTITY)> const
    g_texture_gl_src_types{
        {GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS}
};

static std::array<uint32_t, static_cast<uint32_t>(CombineStage::OperandType::QUANTITY)> const
    g_texture_gl_operand_types{
        {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}
};

// Texture formats mapping
struct GLTextureFormatMapping
{
    GLint    gl_internal_format;
    uint32_t gl_input_format;
    uint32_t gl_input_data_type;
};

static std::array<GLTextureFormatMapping, static_cast<uint32_t>(ImageState::Format::QUANTITY)> const
    g_texture_gl_formats{
        {
         {0, 0, 0}, // NOFORMAT
 {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE}, // R8G8B8
 {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}, // R8G8B8A8
 {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, GL_UNSIGNED_BYTE}, // DXT1
 {GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_UNSIGNED_BYTE}, // DXT3
 {GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, GL_UNSIGNED_BYTE}, // DXT5
 {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT}       // DEPTH
        }
};

constexpr static bool IsCompressedTextureFormat(ImageState::Format fmt)
{
    return (fmt == ImageState::Format::DXT1) || (fmt == ImageState::Format::DXT3)
           || (fmt == ImageState::Format::DXT5);
}

constexpr static glm::vec4 GetMtrxRow(glm::mat4 const & mtx, int32_t row_num = 0)
{
    assert(row_num < 4 && row_num >= 0);

    return glm::vec4(mtx[0][row_num], mtx[1][row_num], mtx[2][row_num], mtx[3][row_num]);
}

bool RendererBase::init()
{
    // Initialize GLEW
    GLenum glew_init_res = glewInit();
    // GLEW_ERROR_NO_GLX_DISPLAY ignored under Wayland as a workaround
    // https://github.com/nigels-com/glew/issues/172
    if(!(glew_init_res == GLEW_OK
         || (glew_init_res == GLEW_ERROR_NO_GLX_DISPLAY && (getenv("WAYLAND_DISPLAY") != nullptr))))
    {
        throw std::runtime_error{"Failed to initialize GLEW"};
    }
    if(!checkExtensions())
    {
        throw std::runtime_error{"Failed to initialize extensions"};
    }

    m_vendor   = reinterpret_cast<char const *>(glGetString(GL_VENDOR));
    m_renderer = reinterpret_cast<char const *>(glGetString(GL_RENDERER));
    m_version  = reinterpret_cast<char const *>(glGetString(GL_VERSION));

    commitAllStates();
    clearBuffers();

    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // https://www.g-truc.net/post-0256.html
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glEnable(GL_NORMALIZE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLint default_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &default_fbo);
    m_default_fbo = static_cast<uint32_t>(default_fbo);

    assert(m_custom_fbo == 0);
    glGenFramebuffersEXT(1, &m_custom_fbo);

    GLint max_lights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
    m_max_lights = static_cast<uint32_t>(max_lights);

    // bbox
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 1.0f,  0.5f, -0.5f, -0.5f, 1.0f, 0.5f, 0.5f, -0.5f,
        1.0f,  -0.5f, 0.5f,  -0.5f, 1.0f, -0.5f, -0.5f, 0.5f, 1.0f, 0.5f, -0.5f,
        0.5f,  1.0f,  0.5f,  0.5f,  0.5f, 1.0f,  -0.5f, 0.5f, 0.5f, 1.0f,
    };

    uint16_t elements[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7};

    glGenBuffers(1, &m_bbox_vbo_vertices);
    glGenBuffers(1, &m_bbox_ibo_elements);

    glBindBuffer(GL_ARRAY_BUFFER, m_bbox_vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bbox_ibo_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    uint8_t const tex_data[] = {128, 192, 255, 255, 128, 192, 255, 255, 255, 192, 128, 255, 255,
                                192, 128, 255, 128, 192, 255, 255, 128, 192, 255, 255, 255, 192,
                                128, 255, 255, 192, 128, 255, 255, 192, 128, 255, 255, 192, 128,
                                255, 128, 192, 255, 255, 128, 192, 255, 255, 255, 192, 128, 255,
                                255, 192, 128, 255, 128, 192, 255, 255, 128, 192, 255, 255};

    glGenTextures(1, &m_default_texture);
    glBindTexture(GL_TEXTURE_2D, m_default_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLint max_texture_units = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
    m_max_texture_slots = static_cast<uint32_t>(max_texture_units);
    m_texture_slots.reserve(m_max_texture_slots);

    GLint max_clip_planes = 0;
    glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
    m_max_clip_planes = static_cast<uint32_t>(max_clip_planes);

    m_initialized = true;

    return true;
}

void RendererBase::terminate()
{
    if(m_initialized)
    {
        glDeleteBuffers(1, &m_bbox_vbo_vertices);
        glDeleteBuffers(1, &m_bbox_ibo_elements);
        m_bbox_vbo_vertices = m_bbox_ibo_elements = 0;

        glDeleteTextures(1, &m_default_texture);
        m_default_texture = 0;

        // destroy FrameBuffer
        if(m_custom_fbo_depth != 0)
        {
            glDeleteTextures(1, &m_custom_fbo_depth);
            m_custom_fbo_depth = 0;
        }
        glDeleteFramebuffersEXT(1, &m_custom_fbo);
        m_custom_fbo = 0;

        m_initialized = false;
    }
}

bool RendererBase::checkExtensions() const
{
    if(!GLEW_ARB_vertex_buffer_object)
    {
        return false;
    }
    else if(!GLEW_ARB_framebuffer_object)
    {
        return false;
    }
    /*else if(!GLEW_ARB_texture_filter_anisotropic)
    {
        return false;
    }*/
    else if(!GLEW_ARB_texture_env_combine)
    {
        return false;
    }
    else if(!GLEW_ARB_texture_float)
    {
        return false;
    }
    else if(!GLEW_ARB_texture_compression)
    {
        return false;
    }
    else if(!GLEW_ARB_depth_texture)
    {
        return false;
    }
    else if(!GLEW_ARB_shadow)
    {
        return false;
    }

    return true;
}

void RendererBase::setMatrix(MatrixType type, glm::mat4 const & matrix) const
{
    GLenum const matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadMatrixf(glm::value_ptr(matrix));
}

void RendererBase::setIdentityMatrix(MatrixType type) const
{
    GLenum const matrix_type = (type == MatrixType::PROJECTION) ? GL_PROJECTION : GL_MODELVIEW;

    glMatrixMode(matrix_type);
    glLoadIdentity();
}

void RendererBase::uploadBuffer(VertexBuffer & geo) const
{
    assert(geo.m_state == VertexBuffer::State::INITDATA);

    if(!geo.m_is_generated)
        glGenBuffers(1, &geo.m_dynamic_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER_ARB, geo.m_dynamic_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * geo.m_dynamic_buffer.size(), &geo.m_dynamic_buffer[0],
                 GL_DYNAMIC_DRAW);

    if(geo.m_components[VertexBuffer::ComponentsBitPos::tex])
    {
        if(!geo.m_is_generated)
            glGenBuffers(1, &geo.m_static_bufffer_id);
        glBindBuffer(GL_ARRAY_BUFFER_ARB, geo.m_static_bufffer_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * geo.m_static_bufffer.size(), &geo.m_static_bufffer[0],
                     GL_STATIC_DRAW);
    }

    if(!geo.m_is_generated)
    {
        glGenBuffers(1, &geo.m_indices_id);
        geo.m_is_generated = true;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo.m_indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * geo.m_indices.size(), &geo.m_indices[0],
                 GL_STATIC_DRAW);

    geo.m_state = VertexBuffer::State::COMMITTED;
}

void RendererBase::unloadBuffer(VertexBuffer const & geo) const
{
    if(geo.m_is_generated)
    {
        glBindBuffer(GL_ARRAY_BUFFER_ARB, geo.m_dynamic_buffer_id);
        glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
        if(geo.m_components[VertexBuffer::ComponentsBitPos::tex])
        {
            glBindBuffer(GL_ARRAY_BUFFER_ARB, geo.m_static_bufffer_id);
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo.m_indices_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
    }
}

void RendererBase::deleteBuffer(VertexBuffer & geo) const
{
    if(geo.m_is_generated)
    {
        glDeleteBuffers(1, &geo.m_dynamic_buffer_id);
        geo.m_dynamic_buffer_id = 0;
        if(geo.m_components[VertexBuffer::ComponentsBitPos::tex])
        {
            glDeleteBuffers(1, &geo.m_static_bufffer_id);
            geo.m_static_bufffer_id = 0;
        }
        glDeleteBuffers(1, &geo.m_indices_id);
        geo.m_indices_id = 0;

        geo.m_is_generated = false;
        geo.m_state        = VertexBuffer::State::NODATA;
    }
}

void RendererBase::bindVertexBuffer(VertexBuffer const * geo) const
{
    if(geo != nullptr)
    {
        if(geo->m_state == VertexBuffer::State::COMMITTED)
        {
            glBindBuffer(GL_ARRAY_BUFFER, geo->m_dynamic_buffer_id);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, 0, static_cast<void *>(nullptr));

            if(geo->m_components[VertexBuffer::ComponentsBitPos::normal])
            {
                glEnableClientState(GL_NORMAL_ARRAY);
                glNormalPointer(GL_FLOAT, 0,
                                reinterpret_cast<void *>(sizeof(float) * geo->m_vertex_count * 3));
            }

            if(geo->m_components[VertexBuffer::ComponentsBitPos::tex])
            {
                for(uint32_t i = 0; i < m_texture_slots.size(); ++i)
                {
                    if(m_texture_slots[i].coord_source == TextureSlot::TexCoordSource::TEX_COORD_BUFFER)
                    {
                        uint32_t tex_coord_start = static_cast<uint32_t>(sizeof(float))
                                                   * m_texture_slots[i].tex_channel_num * geo->m_vertex_count
                                                   * 2;
                        glBindBuffer(GL_ARRAY_BUFFER, geo->m_static_bufffer_id);

                        uint32_t const texture_slot_id = GL_TEXTURE0 + i;
                        glClientActiveTexture(texture_slot_id);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<void *>(tex_coord_start));
                    }
                }
            }

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->m_indices_id);

            m_last_binded_vbo_components = geo->m_components;
        }
    }
}

void RendererBase::unbindVertexBuffer() const
{
    if(m_last_binded_vbo_components != VertexBuffer::null)
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        if(m_last_binded_vbo_components[VertexBuffer::ComponentsBitPos::tex])
        {
            for(uint32_t i = 0; i < m_texture_slots.size(); ++i)
            {
                if(m_texture_slots[i].coord_source == TextureSlot::TexCoordSource::TEX_COORD_BUFFER)
                {
                    uint32_t const texture_slot_id = GL_TEXTURE0 + i;
                    glClientActiveTexture(texture_slot_id);
                    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                }
            }
        }
        if(m_last_binded_vbo_components[VertexBuffer::ComponentsBitPos::normal])
            glDisableClientState(GL_NORMAL_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

        m_last_binded_vbo_components = VertexBuffer::null;
    }
}

void RendererBase::draw(VertexBuffer const & geo) const
{
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(geo.m_indices.size()), GL_UNSIGNED_INT,
                   static_cast<void *>(nullptr));
}

void RendererBase::drawIndexed(uint32_t first_index, uint32_t num_indices, uint32_t first_vert,
                               uint32_t num_verts) const
{
    void * offset = reinterpret_cast<void *>(static_cast<uintptr_t>(first_index) * sizeof(uint32_t));
    glDrawRangeElements(GL_TRIANGLES, first_vert, first_vert + num_verts, num_indices, GL_UNSIGNED_INT,
                        offset);
}

void RendererBase::createTexture(ImageState & tex) const
{
    assert(tex.m_render_id == 0 && tex.m_type != ImageState::Type::TEXTURE_NOTYPE);

    uint32_t const tex_type = g_texture_gl_types[static_cast<uint32_t>(tex.m_type)];

    glGenTextures(1, &tex.m_render_id);
    glBindTexture(tex_type, tex.m_render_id);

    applySamplerState(tex);

    glBindTexture(tex_type, 0);
}

void RendererBase::uploadTextureData(ImageState & tex, tex::ImageData const & tex_data,
                                     ImageState::CubeFace face) const
{
    assert(tex.m_render_id != 0 && tex.m_type != ImageState::Type::TEXTURE_NOTYPE);
    assert(tex_data.data.get() != nullptr);
    assert(tex.m_width == tex_data.width && tex.m_height == tex_data.height && tex.m_depth == tex_data.depth);
    assert(static_cast<int>(tex.m_format) < static_cast<int>(ImageState::Format::QUANTITY));

    uint32_t const  tex_type  = g_texture_gl_types[static_cast<uint32_t>(tex.m_type)];
    uint8_t const * data      = tex_data.data.get();
    GLsizei const   data_size = static_cast<GLsizei>(tex_data.data_size);

    glBindTexture(tex_type, tex.m_render_id);

    bool const compressed = IsCompressedTextureFormat(tex.m_format);

    GLint const internal_format =
        g_texture_gl_formats[static_cast<uint32_t>(tex.m_format)].gl_internal_format;
    uint32_t const input_format = g_texture_gl_formats[static_cast<uint32_t>(tex.m_format)].gl_input_format;
    uint32_t const input_type = g_texture_gl_formats[static_cast<uint32_t>(tex.m_format)].gl_input_data_type;

    if(tex.m_type == ImageState::Type::TEXTURE_2D || tex.m_type == ImageState::Type::TEXTURE_CUBE)
    {
        uint32_t const target = (tex.m_type == ImageState::Type::TEXTURE_2D)
                                    ? tex_type
                                    : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(face));

        if(compressed)
            glCompressedTexImage2D(target, 0, internal_format, tex.m_width, tex.m_height, 0, data_size, data);
        else
            glTexImage2D(target, 0, internal_format, static_cast<int32_t>(tex.m_width),
                         static_cast<int32_t>(tex.m_height), 0, input_format, input_type, data);
    }
    else if(tex.m_type == ImageState::Type::TEXTURE_3D)
    {
        if(compressed)
            glCompressedTexImage3D(GL_TEXTURE_3D, 0, internal_format, tex.m_width, tex.m_height, tex.m_depth,
                                   0, data_size, data);
        else
            glTexImage3D(GL_TEXTURE_3D, 0, internal_format, tex.m_width, tex.m_height, tex.m_depth, 0,
                         input_format, input_type, data);
    }

    if(tex.m_gen_mips
       && (tex.m_type != ImageState::Type::TEXTURE_CUBE || face == ImageState::CubeFace::NEG_Z))
    {
        // Note: for cube maps mips are only generated when the side with the highest index is uploaded
        glEnable(tex_type);
        glGenerateMipmapEXT(tex_type);
        glDisable(tex_type);
    }

    glBindTexture(tex_type, 0);

    tex.m_committed = true;
}

void RendererBase::destroyTexture(ImageState & tex) const
{
    assert(tex.m_render_id != 0);

    glDeleteTextures(1, &tex.m_render_id);
    tex.m_render_id = 0;
    tex.m_committed = false;
}

bool RendererBase::get2DTextureData(ImageState const & tex, tex::ImageData & tex_data,
                                    ImageState::CubeFace face) const
{
    assert(tex.m_render_id != 0
           && (tex.m_type == ImageState::Type::TEXTURE_2D || tex.m_type == ImageState::Type::TEXTURE_CUBE));

    uint32_t target = tex.m_type == ImageState::Type::TEXTURE_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    if(target == GL_TEXTURE_CUBE_MAP)
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(face);

    if(static_cast<uint32_t>(tex.m_format) > g_texture_gl_formats.size())
        return false;

    uint32_t const fmt  = g_texture_gl_formats[static_cast<uint32_t>(tex.m_format)].gl_input_format;
    uint32_t const type = g_texture_gl_formats[static_cast<uint32_t>(tex.m_format)].gl_input_data_type;

    uint32_t const bind_type = g_texture_gl_types[static_cast<uint32_t>(tex.m_type)];
    glBindTexture(bind_type, tex.m_render_id);

    GLint result = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_COMPRESSED, &result);
    bool const compressed = result == GL_TRUE;

    uint32_t                  data_size = 0;
    tex::ImageData::PixelType px_type   = tex::ImageData::PixelType::pt_none;
    if(compressed)
    {
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &result);
        data_size = static_cast<uint32_t>(result);
        px_type   = tex::ImageData::PixelType::pt_compressed;
    }
    else
    {
        // If texture image does not contain four components, the mappings are applied.
        // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
        px_type   = tex::ImageData::PixelType::pt_rgba;
        data_size = tex.m_width * tex.m_height * 4;
    }

    tex_data.width     = tex.m_width;
    tex_data.height    = tex.m_height;
    tex_data.depth     = 0;
    tex_data.data_size = data_size;
    tex_data.type      = px_type;
    tex_data.data      = std::make_unique<uint8_t[]>(tex_data.data_size);

    if(compressed)
        glGetCompressedTexImage(target, 0, tex_data.data.get());
    else
        glGetTexImage(target, 0, fmt, type, tex_data.data.get());

    glBindTexture(bind_type, 0);
    return true;
}

void RendererBase::applySamplerState(ImageState const & tex) const
{
    uint32_t const target = g_texture_gl_types[static_cast<uint32_t>(tex.m_type)];

    if(tex.m_sampler.s == ImageState::Wrap::CLAMP_TO_BORDER)
        glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(tex.m_sampler.border_color));

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, g_gl_tex_filter[static_cast<uint32_t>(tex.m_sampler.min)]);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, g_gl_tex_filter[static_cast<uint32_t>(tex.m_sampler.max)]);
    glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, tex.m_sampler.max_anisotropy);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, g_gl_tex_wrap[static_cast<uint32_t>(tex.m_sampler.s)]);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, g_gl_tex_wrap[static_cast<uint32_t>(tex.m_sampler.t)]);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, g_gl_tex_wrap[static_cast<uint32_t>(tex.m_sampler.r)]);

    if(!tex.m_sampler.compare_mode)
    {
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }
    else if(tex.m_type == ImageState::Type::TEXTURE_2D)
    {
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glTexParameteri(target, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    }
}

// https://www.khronos.org/opengl/wiki/Texture_Combiners
void RendererBase::applyCombineStage(CombineStage const & combine) const
{
    assert(combine.mode != CombineStage::CombineMode::QUANTITY);

    int32_t const combine_func =
        static_cast<int32_t>(g_texture_gl_combine_modes[static_cast<uint32_t>(combine.mode)]);

    if(combine.mode != CombineStage::CombineMode::COMBINE)
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, combine_func);
    }
    else
    {
        int32_t const combine_rgb =
            static_cast<int32_t>(g_texture_gl_combine_functions[static_cast<uint32_t>(combine.rgb_func)]);
        int32_t const combine_alpha =
            static_cast<int32_t>(g_texture_gl_combine_functions[static_cast<uint32_t>(combine.alpha_func)]);

        auto getSrcType = [](CombineStage::SrcType src_type, uint32_t num_stage) {
            uint32_t src = 0;
            if(src_type == CombineStage::SrcType::TEXTURE_STAGE)
            {
                src = g_texture_gl_src_types[static_cast<uint32_t>(src_type)];
                src += num_stage;
            }
            else
            {
                src = g_texture_gl_src_types[static_cast<uint32_t>(src_type)];
            }

            return static_cast<int32_t>(src);
        };

        int32_t const rgb_src0   = getSrcType(combine.rgb_src0, combine.rgb_stage0);
        int32_t const rgb_src1   = getSrcType(combine.rgb_src1, combine.rgb_stage1);
        int32_t const rgb_src2   = getSrcType(combine.rgb_src2, combine.rgb_stage2);
        int32_t const alpha_src0 = getSrcType(combine.alpha_src0, combine.alpha_stage0);
        int32_t const alpha_src1 = getSrcType(combine.alpha_src1, combine.alpha_stage1);
        int32_t const alpha_src2 = getSrcType(combine.alpha_src2, combine.alpha_stage2);

        int32_t const operand_rgb0 =
            static_cast<int32_t>(g_texture_gl_operand_types[static_cast<uint32_t>(combine.rgb_operand0)]);
        int32_t const operand_rgb1 =
            static_cast<int32_t>(g_texture_gl_operand_types[static_cast<uint32_t>(combine.rgb_operand1)]);
        int32_t const operand_rgb2 =
            static_cast<int32_t>(g_texture_gl_operand_types[static_cast<uint32_t>(combine.rgb_operand2)]);
        int32_t const operand_alpha0 =
            static_cast<int32_t>(g_texture_gl_operand_types[static_cast<uint32_t>(combine.alpha_operand0)]);
        int32_t const operand_alpha1 =
            static_cast<int32_t>(g_texture_gl_operand_types[static_cast<uint32_t>(combine.alpha_operand1)]);
        int32_t const operand_alpha2 =
            static_cast<int32_t>(g_texture_gl_operand_types[static_cast<uint32_t>(combine.alpha_operand2)]);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, combine_func);
        // Sample RGB
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, combine_rgb);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, rgb_src0);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, rgb_src1);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, rgb_src2);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, operand_rgb0);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, operand_rgb1);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, operand_rgb2);
        // Sample ALPHA
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, combine_alpha);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, alpha_src0);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, alpha_src1);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, alpha_src2);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, operand_alpha0);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, operand_alpha1);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, operand_alpha2);

        if(combine.rgb_scale != 0)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, combine.rgb_scale);
        }

        if(combine.alpha_scale != 0)
        {
            glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, combine.alpha_scale);
        }

        if(combine.const_color_enabled)
        {
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, glm::value_ptr(combine.constant_color));
        }
    }
}

uint32_t RendererBase::addTextureSlot(TextureSlot slot)
{
    assert(m_texture_slots.size() < m_max_texture_slots);

    m_texture_slots.push_back(std::move(slot));
    return static_cast<uint32_t>(m_texture_slots.size() - 1);
}

TextureSlot & RendererBase::getTextureSlot(uint32_t slot_num)
{
    assert(slot_num < m_texture_slots.size());

    return m_texture_slots[slot_num];
}

void RendererBase::bindSlots() const
{
    for(uint32_t i = 0; i < m_texture_slots.size(); ++i)
    {
        if(m_texture_slots[i].coord_source == TextureSlot::TexCoordSource::TEX_COORD_BUFFER)
        {
            uint32_t const texture_slot_id = GL_TEXTURE0 + i;
            uint32_t const target =
                g_texture_gl_types[static_cast<uint32_t>(m_texture_slots[i].texture->m_type)];
            glActiveTexture(texture_slot_id);
            glEnable(target);
            glBindTexture(target, m_texture_slots[i].texture->m_render_id);
        }
        else
        {
            uint32_t const target = g_texture_gl_types[static_cast<uint32_t>(
                m_texture_slots[i].projector->projected_texture->m_type)];
            enableTextureCoordGeneration(i, target);
        }

        applyCombineStage(m_texture_slots[i].combine_mode);
    }
}

void RendererBase::unbindSlots() const
{
    for(uint32_t i = 0; i < m_texture_slots.size(); ++i)
    {
        if(m_texture_slots[i].coord_source == TextureSlot::TexCoordSource::TEX_COORD_BUFFER)
        {
            uint32_t const texture_slot_id = GL_TEXTURE0 + i;
            uint32_t const target =
                g_texture_gl_types[static_cast<uint32_t>(m_texture_slots[i].texture->m_type)];
            glActiveTexture(texture_slot_id);
            glBindTexture(target, 0);
            glDisable(target);
        }
        else
        {
            uint32_t const target = g_texture_gl_types[static_cast<uint32_t>(
                m_texture_slots[i].projector->projected_texture->m_type)];
            disableTextureCoordGeneration(i, target);
        }

        applyCombineStage({});
    }
}

void RendererBase::clearSlots()
{
    m_texture_slots.resize(0);
}

void RendererBase::unbindAndClearSlots()
{
    unbindSlots();
    clearSlots();
}

void RendererBase::enableTextureCoordGeneration(std::uint32_t slot_num, uint32_t target) const
{
    assert(slot_num < m_texture_slots.size());

    TextureSlot const & slot = m_texture_slots[slot_num];
    assert(slot.projector != nullptr);

    glActiveTexture(GL_TEXTURE0 + slot_num);
    glEnable(target);
    glBindTexture(target, slot.projector->projected_texture->m_render_id);

    if(!slot.projector->is_cube_map)
    {
        assert(slot.projector->projected_texture->m_type == ImageState::Type::TEXTURE_2D);

        auto transform_mtx = slot.projector->getTransformMatrix();

        // Set up texture coordinate generation
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_S, GL_EYE_PLANE, glm::value_ptr(GetMtrxRow(transform_mtx, 0)));
        glEnable(GL_TEXTURE_GEN_S);

        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_T, GL_EYE_PLANE, glm::value_ptr(GetMtrxRow(transform_mtx, 1)));
        glEnable(GL_TEXTURE_GEN_T);

        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_R, GL_EYE_PLANE, glm::value_ptr(GetMtrxRow(transform_mtx, 2)));
        glEnable(GL_TEXTURE_GEN_R);

        glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
        glTexGenfv(GL_Q, GL_EYE_PLANE, glm::value_ptr(GetMtrxRow(transform_mtx, 3)));
        glEnable(GL_TEXTURE_GEN_Q);
    }
    else
    {
        assert(slot.projector->projected_texture->m_type == ImageState::Type::TEXTURE_CUBE);

        int32_t refl_mode =
            slot.cube_map_mode == TextureSlot::CubeMapGenMode::NORMAL ? GL_NORMAL_MAP : GL_REFLECTION_MAP;

        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, refl_mode);
        glEnable(GL_TEXTURE_GEN_S);

        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, refl_mode);
        glEnable(GL_TEXTURE_GEN_T);

        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, refl_mode);
        glEnable(GL_TEXTURE_GEN_R);
    }
}

void RendererBase::disableTextureCoordGeneration(std::uint32_t slot_num, uint32_t target) const
{
    assert(slot_num < m_texture_slots.size());

    glActiveTexture(GL_TEXTURE0 + slot_num);
    glDisable(target);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
    glBindTexture(target, 0);
}

void RendererBase::clearLight(uint32_t index)
{
    assert(m_lights_queue.size() > index);

    auto const it_pos = m_lights_queue.begin() + index;
    m_lights_queue.erase(it_pos);
}

uint32_t RendererBase::addLight(Light light)
{
    if(m_lights_queue.size() < m_max_lights)
    {
        m_lights_queue.push_back(std::move(light));
        return static_cast<uint32_t>(m_lights_queue.size() - 1);
    }

    return static_cast<uint32_t>(-1);
}

void RendererBase::bindLights() const
{
    if(m_lights_queue.size() > 0)
        glEnable(GL_LIGHTING);

    for(uint32_t light_num = 0; light_num < m_lights_queue.size(); ++light_num)
    {
        auto const & light         = m_lights_queue[light_num];
        GLenum const light_src_num = GL_LIGHT0 + light_num;

        glLightfv(light_src_num, GL_POSITION, glm::value_ptr(light.m_position));
        glLightfv(light_src_num, GL_AMBIENT, glm::value_ptr(light.m_ambient));
        glLightfv(light_src_num, GL_DIFFUSE, glm::value_ptr(light.m_diffuse));
        glLightfv(light_src_num, GL_SPECULAR, glm::value_ptr(light.m_specular));

        if(light.m_type == Light::LightType::Point || light.m_type == Light::LightType::Spot)
        {
            if(light.m_range > 0.f)
            {
                glLightf(light_src_num, GL_CONSTANT_ATTENUATION, 0.0f);
                glLightf(light_src_num, GL_LINEAR_ATTENUATION, 5.0f / light.m_range);
                glLightf(light_src_num, GL_QUADRATIC_ATTENUATION, 0.0f);
            }
            else
            {
                glLightf(light_src_num, GL_CONSTANT_ATTENUATION, 1.0f);
                glLightf(light_src_num, GL_LINEAR_ATTENUATION, 0.0f);
                glLightf(light_src_num, GL_QUADRATIC_ATTENUATION, 0.0f);
            }
        }

        if(light.m_type == Light::LightType::Spot)
        {
            float const angle = glm::degrees(glm::acos(light.m_spot_cos_cutoff));

            glLightf(light_src_num, GL_SPOT_CUTOFF, angle);
            glLightfv(light_src_num, GL_SPOT_DIRECTION, glm::value_ptr(light.m_spot_direction));
            glLightf(light_src_num, GL_SPOT_EXPONENT, light.m_spot_exponent);
        }
        // Enable light source
        glEnable(light_src_num);
    }
}

void RendererBase::unbindLights() const
{
    for(uint32_t light_num = 0; light_num < m_lights_queue.size(); ++light_num)
    {
        GLenum const light_src_num = GL_LIGHT0 + light_num;
        auto const & light         = m_lights_queue[light_num];

        // reset to default
        if(light.m_type == Light::LightType::Point || light.m_type == Light::LightType::Spot)
        {
            glLightf(light_src_num, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(light_src_num, GL_LINEAR_ATTENUATION, 0.0f);
            glLightf(light_src_num, GL_QUADRATIC_ATTENUATION, 0.0f);
        }

        if(light.m_type == Light::LightType::Spot)
        {
            glLightf(light_src_num, GL_SPOT_CUTOFF, 180.0f);
            glLightfv(light_src_num, GL_SPOT_DIRECTION, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
            glLightf(light_src_num, GL_SPOT_EXPONENT, 0.0f);
        }

        glDisable(light_src_num);
    }

    if(m_lights_queue.size() > 0)
        glDisable(GL_LIGHTING);
}

// https://www.khronos.org/opengl/wiki/Framebuffer_Object_Extension_Examples
bool RendererBase::bindTextureAsFrameBuffer(ImageState * color_tex, ImageState * depth_tex,
                                            glm::ivec4 viewport_size)
{
    assert(m_custom_fbo != 0);
    assert(color_tex != nullptr || depth_tex != nullptr);

    if(color_tex != nullptr && depth_tex != nullptr)
    {
        if(color_tex->m_width != depth_tex->m_width || color_tex->m_height != depth_tex->m_height)
            return false;
    }

    uint32_t loc_width  = 0;
    uint32_t loc_height = 0;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_custom_fbo);

    if(color_tex != nullptr)
    {
        assert(m_fbo_color_attached == false);

        if(color_tex->m_format != ImageState::Format::R8G8B8A8)
            return false;

        GLint const internal_format =
            g_texture_gl_formats[static_cast<uint32_t>(color_tex->m_format)].gl_internal_format;
        uint32_t const input_format =
            g_texture_gl_formats[static_cast<uint32_t>(color_tex->m_format)].gl_input_format;
        uint32_t const input_type =
            g_texture_gl_formats[static_cast<uint32_t>(color_tex->m_format)].gl_input_data_type;

        loc_width  = color_tex->m_width;
        loc_height = color_tex->m_height;

        if(color_tex->m_render_id == 0)
            glGenTextures(1, &color_tex->m_render_id);
        glBindTexture(GL_TEXTURE_2D, color_tex->m_render_id);
        applySamplerState(*color_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, static_cast<GLsizei>(loc_width),
                     static_cast<GLsizei>(loc_height), 0, input_format, input_type, nullptr);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D,
                                  color_tex->m_render_id, 0);
        m_fbo_color_attached = true;

        GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, draw_buffers);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    if(depth_tex != nullptr)
    {
        GLint const internal_format =
            g_texture_gl_formats[static_cast<uint32_t>(depth_tex->m_format)].gl_internal_format;
        uint32_t const input_format =
            g_texture_gl_formats[static_cast<uint32_t>(depth_tex->m_format)].gl_input_format;
        uint32_t const input_type =
            g_texture_gl_formats[static_cast<uint32_t>(depth_tex->m_format)].gl_input_data_type;

        loc_width  = depth_tex->m_width;
        loc_height = depth_tex->m_height;

        if(depth_tex->m_render_id == 0)
            glGenTextures(1, &depth_tex->m_render_id);
        glBindTexture(GL_TEXTURE_2D, depth_tex->m_render_id);
        applySamplerState(*depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, static_cast<GLsizei>(loc_width),
                     static_cast<GLsizei>(loc_height), 0, input_format, input_type, nullptr);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D,
                                  depth_tex->m_render_id, 0);
    }
    else
    {
        if(m_custom_fbo_depth == 0)
            glGenTextures(1, &m_custom_fbo_depth);
        glBindTexture(GL_TEXTURE_2D, m_custom_fbo_depth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(loc_width),
                     static_cast<GLsizei>(loc_height), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D,
                                  m_custom_fbo_depth, 0);
    }

    // Check if successful
    uint32_t status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        return false;
    }

    if(color_tex != nullptr)
    {
        color_tex->m_committed = true;
    }

    if(depth_tex != nullptr)
    {
        depth_tex->m_committed = true;
    }

    if(glm::all(glm::notEqual(viewport_size, glm::ivec4{0})))
    {
        glViewport(static_cast<GLint>(viewport_size.x), static_cast<GLint>(viewport_size.y),
                   static_cast<GLsizei>(viewport_size.z), static_cast<GLsizei>(viewport_size.w));
    }
    else
        glViewport(0, 0, static_cast<GLsizei>(loc_width), static_cast<GLsizei>(loc_height));

    return true;
}

void RendererBase::unbindTexturesFromFrameBuffer() const
{
    if(m_fbo_color_attached)
    {
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
        m_fbo_color_attached = false;
    }
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, 0, 0);
}

void RendererBase::bindDefaultFbo()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_default_fbo);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
    glViewport(m_viewport_pos.x, m_viewport_pos.y, m_viewport_size.x, m_viewport_size.y);
}

void RendererBase::enableClipPlane(uint32_t plane_num, glm::vec4 const & plane) const
{
    assert(plane_num < m_max_clip_planes);

    uint32_t plane_id    = GL_CLIP_PLANE0 + plane_num;
    double   plane_ar[4] = {plane.x, plane.y, plane.z, plane.w};

    glEnable(plane_id);
    glClipPlane(plane_id, plane_ar);
}

void RendererBase::disableClipPlane(uint32_t plane_num) const
{
    assert(plane_num < m_max_clip_planes);

    uint32_t plane_id = GL_CLIP_PLANE0 + plane_num;

    glDisable(plane_id);
}

void RendererBase::setDrawColor(glm::vec4 const & color) const
{
    glColor4fv(glm::value_ptr(color));
}

// https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
void RendererBase::drawBBox(AABB const & bbox, glm::mat4 const & object2world, glm::vec3 const & color)
{
    glm::vec3 const size   = bbox.max() - bbox.min();
    glm::vec3 const center = (bbox.min() + bbox.max()) / 2.0f;
    glm::mat4 const transform =
        object2world * glm::translate(glm::mat4(1), center) * glm::scale(glm::mat4(1), size);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(glm::value_ptr(transform));

    auto        old_offset = getOffsetState();
    OffsetState temp_offset_state;
    temp_offset_state.fill_enabled  = true;
    temp_offset_state.line_enabled  = true;
    temp_offset_state.point_enabled = true;
    temp_offset_state.scale         = 1.f;
    temp_offset_state.bias          = 0.f;
    setOffsetState(temp_offset_state);

    glLineWidth(2.f);
    glColor3fv(glm::value_ptr(color));

    glBindBuffer(GL_ARRAY_BUFFER, m_bbox_vbo_vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(4,          // number of elements per vertex, here (x,y,z,w));
                    GL_FLOAT,   // the type of each element
                    0,          // no extra data between each position
                    0           // offset of first element
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bbox_ibo_elements);

    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(4 * sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(8 * sizeof(GLushort)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.f);
    setOffsetState(old_offset);

    glPopMatrix();
}

void RendererBase::clearColorBuffer() const
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RendererBase::clearDepthBuffer() const
{
    glClearDepth(m_clear_depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void RendererBase::clearStencilBuffer() const
{
    glClearStencil(m_clear_stencil);
    glClear(GL_STENCIL_BUFFER_BIT);
}

void RendererBase::clearBuffers() const
{
    glClearColor(m_clear_color[0], m_clear_color[1], m_clear_color[2], m_clear_color[3]);
    glClearDepth(m_clear_depth);
    glClearStencil(m_clear_stencil);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RendererBase::setViewport(int32_t x_pos, int32_t y_pos, int32_t width, int32_t height)
{
    glViewport(x_pos, y_pos, width, height);

    m_viewport_pos.x  = x_pos;
    m_viewport_pos.y  = y_pos;
    m_viewport_size.x = width;
    m_viewport_size.y = height;
}

AlphaState RendererBase::setAlphaState(AlphaState const & new_state)
{
    if(m_alpha == new_state)
        return m_alpha;

    AlphaState old = m_alpha;
    m_alpha        = new_state;
    commitAlphaState();

    return old;
}

CullState RendererBase::setCullState(CullState const & new_state)
{
    if(m_cull == new_state)
        return m_cull;

    CullState old = m_cull;
    m_cull        = new_state;
    commitCullState();

    return old;
}

DepthState RendererBase::setDepthState(DepthState const & new_state)
{
    if(m_depth == new_state)
        return m_depth;

    DepthState old = m_depth;
    m_depth        = new_state;
    commitDepthState();

    return old;
}

OffsetState RendererBase::setOffsetState(OffsetState const & new_state)
{
    if(m_offset == new_state)
        return m_offset;

    OffsetState old = m_offset;
    m_offset        = new_state;
    commitOffsetState();

    return old;
}

StencilState RendererBase::setStencilState(StencilState const & new_state)
{
    if(m_stencil == new_state)
        return m_stencil;

    StencilState old = m_stencil;
    m_stencil        = new_state;
    commitStencilState();

    return old;
}

WireState RendererBase::setWireState(WireState const & new_state)
{
    if(m_wire == new_state)
        return m_wire;

    WireState old = m_wire;
    m_wire        = new_state;
    commitWireState();

    return old;
}

void RendererBase::commitAlphaState() const
{
    if(m_alpha.blend_enabled)
    {
        GLenum const src_blend = g_gl_alpha_src_blend[static_cast<uint32_t>(m_alpha.src_blend)];
        GLenum const dst_blend = g_gl_alpha_dst_blend[static_cast<uint32_t>(m_alpha.dst_blend)];

        glEnable(GL_BLEND);
        glBlendFunc(src_blend, dst_blend);
        glBlendColor(m_alpha.constant_color[0], m_alpha.constant_color[1], m_alpha.constant_color[2],
                     m_alpha.constant_color[3]);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if(m_alpha.compare_enabled)
    {
        GLenum const compare = g_gl_compare_mode[static_cast<uint32_t>(m_alpha.compare)];

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(compare, m_alpha.reference);
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
    }
}

void RendererBase::commitCullState() const
{
    if(m_cull.enabled)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        bool order = m_cull.ccw_order;
        if(order)
            glCullFace(GL_BACK);
        else
            glCullFace(GL_FRONT);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void RendererBase::commitDepthState() const
{
    if(m_depth.enabled)
    {
        GLenum const compare = g_gl_compare_mode[static_cast<uint32_t>(m_depth.compare)];

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(compare);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    if(m_depth.writable)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);
}

void RendererBase::commitOffsetState() const
{
    if(m_offset.fill_enabled)
        glEnable(GL_POLYGON_OFFSET_FILL);
    else
        glDisable(GL_POLYGON_OFFSET_FILL);

    if(m_offset.line_enabled)
        glEnable(GL_POLYGON_OFFSET_LINE);
    else
        glDisable(GL_POLYGON_OFFSET_LINE);

    if(m_offset.point_enabled)
        glEnable(GL_POLYGON_OFFSET_POINT);
    else
        glDisable(GL_POLYGON_OFFSET_POINT);

    glPolygonOffset(m_offset.scale, m_offset.bias);
}

void RendererBase::commitStencilState() const
{
    if(m_stencil.enabled)
    {
        glEnable(GL_STENCIL_TEST);

        GLenum const compare   = g_gl_compare_mode[static_cast<uint32_t>(m_stencil.compare)];
        GLenum const on_fail   = g_gl_stencil_operation[static_cast<uint32_t>(m_stencil.on_fail)];
        GLenum const on_z_fail = g_gl_stencil_operation[static_cast<uint32_t>(m_stencil.on_z_fail)];
        GLenum const on_z_pass = g_gl_stencil_operation[static_cast<uint32_t>(m_stencil.on_z_pass)];

        glStencilFunc(compare, m_stencil.reference, m_stencil.mask);
        glStencilMask(m_stencil.write_mask);
        glStencilOp(on_fail, on_z_fail, on_z_pass);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
}

void RendererBase::commitWireState() const
{
    if(m_wire.enabled)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RendererBase::commitAllStates() const
{
    commitAlphaState();
    commitCullState();
    commitDepthState();
    commitOffsetState();
    commitStencilState();
    commitWireState();
}
