#include "texture.h"
#include "renderer.h"
#include "../fs/file.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 TextureProjector::getTransformMatrix() const
{
    assert(projected_texture != nullptr);
    assert(projected_texture->m_height > 0);

    glm::mat4 reflect = glm::mat4(1.f);

    // clang-format off
    glm::mat4 const bias_matrix(0.5f, 0.0f, 0.0f, 0.0f,
                                0.0f, 0.5f, 0.0f, 0.0f,
                                0.0f, 0.0f, 0.5f, 0.0f,
                                0.5f, 0.5f, 0.5f, 1.0f);
    // clang-format on

    if(is_reflection)
        reflect = reflection;

    return bias_matrix * getProjectionMatrix() * getModelviewMatrix() * reflect;
}

glm::mat4 TextureProjector::getProjectionMatrix() const
{
    assert(projected_texture->m_height > 0);

    glm::mat4 projection_matrix(1.0f);
    float     aspect =
        static_cast<float>(projected_texture->m_width) / static_cast<float>(projected_texture->m_height);

    if(is_ortho)
    {
        if(projected_texture->m_width >= projected_texture->m_height)
        {
            projection_matrix = glm::ortho(-10.0f * aspect, 10.0f * aspect, -10.0f, 10.0f, 0.1f, 100.0f);
        }
        else
        {
            projection_matrix = glm::ortho(-10.0f, 10.0f, -10.0f / aspect, 10.0f / aspect, 0.1f, 100.0f);
        }
    }
    else
        projection_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 100.0f);

    return projection_matrix;
}

glm::mat4 TextureProjector::getModelviewMatrix() const
{
    return modelview;
}

glm::mat4 TextureProjector::GetReflectionMatrix(glm::vec4 const & plane)
{
    glm::vec4 c0(1.f - 2.f * plane.x * plane.x, -2.f * plane.x * plane.y, -2.f * plane.x * plane.z, 0.f),
        c1(-2.f * plane.x * plane.y, 1.f - 2.f * plane.y * plane.y, -2.f * plane.y * plane.z, 0.f),
        c2(-2.f * plane.x * plane.z, -2.f * plane.y * plane.z, 1.f - 2.f * plane.z * plane.z, 0.f),
        c3(-2.f * plane.x * plane.w, -2.f * plane.y * plane.w, -2.f * plane.z * plane.w, 1.f);

    return glm::mat4(c0, c1, c2, c3);
}

glm::vec4 TextureProjector::GetPlaneFromPoints(glm::vec3 const & p0, glm::vec3 const & p1,
                                               glm::vec3 const & p2)
{
    auto vec_a = p1 - p0;
    auto vec_b = p2 - p0;

    auto  norm = glm::normalize(glm::cross(vec_a, vec_b));
    float d    = -(norm.x * p0.x + norm.y * p0.y + norm.z * p0.z);

    return glm::vec4(norm, d);
}

bool Texture::loadImageDataFromFile(std::string const & fname, RendererBase const & render)
{
    assert(is_cube_map == false);

    if(!tex::ReadTGA(fname, image_data))
        return false;

    memory_state = MemoryState::CPU_MEMORY;

    loadImageData(render);

    return true;
}

bool Texture::loadImageDataFromFile(BaseFile const & file, RendererBase const & render)
{
    assert(is_cube_map == false);

    if(!tex::ReadTGA(file, image_data))
        return false;

    memory_state = MemoryState::CPU_MEMORY;

    loadImageData(render);

    return true;
}

void Texture::loadImageData(RendererBase const & render)
{
    assert(is_cube_map == false);
    assert(memory_state == MemoryState::CPU_MEMORY);

    texture_state.m_committed = false;
    texture_state.m_type      = TextureState::Type::TEXTURE_2D;
    texture_state.m_format    = image_data.type == tex::ImageData::PixelType::pt_rgb
                                    ? TextureState::Format::R8G8B8
                                    : TextureState::Format::R8G8B8A8;
    texture_state.m_width     = image_data.width;
    texture_state.m_height    = image_data.height;
    texture_state.m_depth     = 1;

    render.createTexture(texture_state);
    render.uploadTextureData(texture_state, image_data);

    assert(texture_state.m_committed);
    memory_state = MemoryState::CPU_GPU_MEMORY;
}

bool Texture::loadCubeMapFromFiles(std::array<char const *, 6> const & fnames, RendererBase const & render)
{
    assert(memory_state == MemoryState::NONE);

    is_cube_map               = true;
    texture_state.m_committed = false;
    texture_state.m_type      = TextureState::Type::TEXTURE_CUBE;
    texture_state.m_depth     = 0;

    render.createTexture(texture_state);

    for(std::size_t i = 0; i < fnames.size(); ++i)
    {
        if(!tex::ReadTGA(fnames[i], cube_map_faces[i]))
            return false;

        texture_state.m_format = cube_map_faces[i].type == tex::ImageData::PixelType::pt_rgb
                                     ? TextureState::Format::R8G8B8
                                     : TextureState::Format::R8G8B8A8;
        texture_state.m_width  = cube_map_faces[i].width;
        texture_state.m_height = cube_map_faces[i].height;

        render.uploadTextureData(texture_state, cube_map_faces[i], static_cast<TextureState::CubeFace>(i));
    }

    memory_state = MemoryState::CPU_GPU_MEMORY;

    return true;
}
