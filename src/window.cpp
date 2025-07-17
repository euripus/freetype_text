#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>

#include "render/renderer.h"
#include "input/inputglfw.h"
#include "scene_data.h"

namespace
{
constexpr char const * base_tex_fname = "color.tga";
constexpr char const * data_folder    = "./data";
};
}   // namespace

Window::Window(int width, int height, char const * title)
    : m_size{width, height},
      m_title{title},
      m_pyramid{VertexBuffer::pos_norm_tex, 2},
      m_win_buf(VertexBuffer::pos_tex),
      m_text_win_buf(VertexBuffer::pos_tex),
      m_fs(data_folder)
{
    // Initialise GLFW
    if(!glfwInit())
    {
        throw std::runtime_error{"Failed to initialize GLFW"};
    }

    mp_base_video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    m_light.m_type     = Light::LightType::Point;
    m_light.m_range    = 100.f;
    m_light.m_diffuse  = glm::vec4(1.f);
    m_light.m_specular = glm::vec4(1.f);
    m_light.m_ambient  = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    m_light.m_position = glm::vec4(0.f, 4.5f, 3.3f, 1.0f);

    m_base_texture.m_type        = Texture::Type::TEXTURE_2D;
    m_base_texture.m_format      = Texture::Format::R8G8B8A8;
    m_base_texture.m_width       = 1024;
    m_base_texture.m_height      = 1024;
    m_base_texture.m_sampler.max = Texture::Filter::LINEAR;
    m_base_texture.m_sampler.min = Texture::Filter::LINEAR_MIPMAP_LINEAR;
    m_base_texture.m_sampler.r   = Texture::Wrap::REPEAT;
    m_base_texture.m_sampler.s   = Texture::Wrap::REPEAT;
    m_base_texture.m_sampler.t   = Texture::Wrap::REPEAT;
}

Window::~Window()
{
    // Cleanup VBO and textures
    if(mp_glfw_win && m_render_ptr->isInit())
    {
        m_render_ptr->unloadBuffer(m_pyramid);
        m_render_ptr->deleteBuffer(m_pyramid);

        m_render_ptr->unloadBuffer(m_plane);
        m_render_ptr->deleteBuffer(m_plane);

        m_render_ptr->unloadBuffer(m_sphere);
        m_render_ptr->deleteBuffer(m_sphere);

        m_render_ptr->unloadBuffer(m_win_buf);
        m_render_ptr->deleteBuffer(m_win_buf);

        m_render_ptr->unloadBuffer(m_text_win_buf);
        m_render_ptr->deleteBuffer(m_text_win_buf);

        m_render_ptr->destroyTexture(m_base_texture);

        m_render_ptr->clearLights();

        m_render_ptr->terminate();
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void Window::createWindow()
{
    GLFWmonitor * mon;
    if(m_is_fullscreen)
    {
        mon         = glfwGetPrimaryMonitor();
        m_vp_size.x = mp_base_video_mode->width;
        m_vp_size.y = mp_base_video_mode->height;
    }
    else
    {
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        mon         = nullptr;
        m_vp_size.x = m_size.x;
        m_vp_size.y = m_size.y;
    }

    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    GLFWwindow * new_window{nullptr};
    new_window = glfwCreateWindow(m_vp_size.x, m_vp_size.y, "", mon, mp_glfw_win);
    if(mp_glfw_win != nullptr)
        glfwDestroyWindow(mp_glfw_win);

    mp_glfw_win = new_window;
    if(mp_glfw_win == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error{"Failed to create GLFW window"};
    }
    glfwMakeContextCurrent(mp_glfw_win);
    glfwSetWindowTitle(mp_glfw_win, m_title.c_str());

    m_render_ptr = std::make_unique<RendererBase>();

    m_render_ptr->setViewport(0, 0, m_vp_size.x, m_vp_size.y);
    auto prj_mtx = glm::perspective(
        glm::radians(45.0f), static_cast<float>(m_vp_size.x) / static_cast<float>(m_vp_size.y), 0.1f, 100.0f);
    m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(mp_glfw_win, GLFW_STICKY_KEYS, GL_TRUE);

    m_render_ptr->init();

    m_render_ptr->addLight(m_light);

    // input backend
    m_input_ptr = std::make_unique<InputGLFW>(mp_glfw_win);
}

void Window::fullscreen(bool is_fullscreen)
{
    if(is_fullscreen == m_is_fullscreen)
        return;

    m_is_fullscreen = is_fullscreen;
    createWindow();
}

void Window::initScene()
{
    m_pyramid.pushBack(pyr_vertex_buffer_data, {pyr_tex_buffer_data0, pyr_tex_buffer_data1},
                       pyr_normal_buffer_data, sizeof(pyr_vertex_buffer_data) / (sizeof(float) * 3),
                       pyr_index_buffer_data, sizeof(pyr_index_buffer_data) / sizeof(unsigned int));
    m_render_ptr->uploadBuffer(m_pyramid);

    m_plane.pushBack(plane_vertex_buffer_data, {plane_tex_buffer_data}, plane_normal_buffer_data,
                     sizeof(plane_vertex_buffer_data) / (sizeof(float) * 3), plane_index_buffer_data,
                     sizeof(plane_index_buffer_data) / sizeof(unsigned int));
    m_render_ptr->uploadBuffer(m_plane);

    m_sphere.pushBack(sphere_vertex_buffer_data, {sphere_tex_buffer_data}, sphere_normal_buffer_data,
                      sizeof(sphere_vertex_buffer_data) / (sizeof(float) * 3), sphere_index_buffer_data,
                      sizeof(sphere_index_buffer_data) / sizeof(unsigned int));
    m_render_ptr->uploadBuffer(m_sphere);

    // create textures
    if(!m_base_texture.loadImageDataFromFile(base_tex_fname, *m_render_ptr))
        throw std::runtime_error("Texture not found");
}

void Window::run()
{
    glm::mat4   prj_mtx, mtx;
    TextureSlot slot;

    do
    {
        static uint32_t num_frames = 0;
        static double   last_time  = 0.0;

        if(glfwGetTime() - last_time > 1.0)
        {
            last_time  = glfwGetTime();
            g_num_fps  = num_frames;
            num_frames = 0;
        }
        num_frames++;

        m_input_ptr->update();

        //         Render scene:
        // bind lights
        // for each mesh:
        //      set slots
        //      bind textures
        //      bind VBO
        //      draw object
        //      unbind VBO
        //      unbind textures
        //      clear slots
        // unbind lights
        m_render_ptr->setClearColor(glm::vec4(0.0f, 0.0f, 0.4f, 1.0f));
        m_render_ptr->clearBuffers();

        prj_mtx =
            glm::perspective(glm::radians(45.0f),
                             static_cast<float>(m_vp_size.x) / static_cast<float>(m_vp_size.y), 0.1f, 100.0f);
        mtx = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, -7.0f});
        mtx = glm::rotate(mtx, glm::radians(45.0f), {1.0f, 0.0f, 0.0f});
        mtx = glm::rotate(mtx, glm::radians(60.0f), {0.0f, 1.0f, 0.0f});
        m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);
        m_render_ptr->setMatrix(RendererBase::MatrixType::MODELVIEW, mtx);

        m_render_ptr->bindLights();

        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot.tex_channel_num   = 1;
        slot.texture           = &m_base_texture;
        slot.projector         = nullptr;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);
        m_render_ptr->bindSlots();
        m_render_ptr->bindVertexBuffer(&m_pyramid);
        m_render_ptr->draw(m_pyramid);
        m_render_ptr->unbindVertexBuffer();
        m_render_ptr->unbindAndClearSlots();

        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot.tex_channel_num   = 0;
        slot.texture           = &m_base_texture;
        slot.projector         = nullptr;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);
        m_render_ptr->bindSlots();
        m_render_ptr->bindVertexBuffer(&m_plane);
        m_render_ptr->draw(m_plane);
        m_render_ptr->unbindVertexBuffer();
        m_render_ptr->unbindAndClearSlots();

        slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
        slot.tex_channel_num   = 0;
        slot.texture           = &m_base_texture;
        slot.projector         = nullptr;
        slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
        m_render_ptr->addTextureSlot(slot);
        m_render_ptr->bindSlots();
        m_render_ptr->bindVertexBuffer(&m_sphere);
        m_render_ptr->draw(m_sphere);
        m_render_ptr->unbindVertexBuffer();
        m_render_ptr->unbindAndClearSlots();

        m_render_ptr->unbindLights();

        AABB test_box({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});
        m_render_ptr->drawBBox(test_box, glm::mat4(1.f), {1.0f, 0.0f, 0.0f});

        // draw UI
        prj_mtx = glm::ortho(0.0, m_ui.getScreenSize().x, 0, m_ui.getScreenSize().y, -1.0, 1.0);
        m_render_ptr->setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);
        m_render_ptr->setIdentityMatrix(RendererBase::MatrixType::MODELVIEW);

        // Swap buffers
        glfwSwapBuffers(mp_glfw_win);
        glfwPollEvents();

        if(m_input_ptr->isKeyPressed(KeyboardKey::Key_F1))
            key_f1();
    }   // Check if the ESC key was pressed or the window was closed
    while(!m_input_ptr->isKeyPressed(KeyboardKey::Key_Escape) && glfwWindowShouldClose(mp_glfw_win) == 0);
}

void Window::key_f1()
{
    fullscreen(!m_is_fullscreen);
}
