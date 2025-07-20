#ifndef WINDOW_H
#define WINDOW_H

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "input/input.h"
#include "render/vertex_buffer.h"
#include "render/texture.h"
#include "src/gui/ui.h"

class GLFWvidmode;
class GLFWwindow;
class RendererBase;

class Window
{
    // window state
    bool                m_is_fullscreen    = false;
    GLFWvidmode const * mp_base_video_mode = nullptr;
    GLFWwindow *        mp_glfw_win        = nullptr;
    glm::ivec2 const    m_size;   // initial size
    glm::ivec2          m_vp_size;
    std::string         m_title   = "UI example";
    bool                m_wire    = false;
    bool                m_running = true;
    unsigned int        m_num_fps = 0;   // Fps counter

    std::unique_ptr<Input>        m_input_ptr;
    std::unique_ptr<RendererBase> m_render_ptr;
    std::unique_ptr<UI>           m_ui_ptr;

    // Scene
    VertexBuffer m_pyramid;
    VertexBuffer m_plane;
    VertexBuffer m_sphere;
    Texture      m_base_texture;
    Light        m_light;
    VertexBuffer m_win_buf;
    VertexBuffer m_text_win_buf;

    // UI
    UIWindow * m_win = nullptr;

    // Enviroment
    FileSystem m_fs;

    void setUIData(UIWindow * win) const;

public:
    Window(int width, int height, char const * title);
    ~Window();

    Window(Window const &)             = delete;
    Window & operator=(Window const &) = delete;

    bool           isFullscreen() const { return m_is_fullscreen; }
    glm::ivec2     getWindowSize() const { return m_vp_size; }
    Input &        getInputMgr() { return *m_input_ptr; }
    RendererBase & getRenderer() { return *m_render_ptr; }
    UI &           getUIMgr() { return *m_ui_ptr; }

    void createWindow();
    void initScene();
    void fullscreen(bool is_fullscreen);
    void run();
    void resize(int width, int height);

    // keys
    void key_f1();
};

#endif   // WINDOW_H
