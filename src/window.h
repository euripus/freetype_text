#ifndef WINDOW_H
#define WINDOW_H

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "input/input.h"
#include "render/vertex_buffer.h"
#include "render/texture.h"

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
    std::string         m_title = "UI example";
	bool         m_wire    = false;
    unsigned int m_num_fps = 0;   // Fps counter

    std::unique_ptr<Input>        m_input_ptr;
    std::unique_ptr<RendererBase> m_render_ptr;

    // Scene
    VertexBuffer     m_pyramid;
    VertexBuffer     m_plane;
    VertexBuffer     m_sphere;
    Texture          m_base_texture;
    Light            m_light;
	std::unique_ptr<UI>           	 m_ui_ptr;
	VertexBuffer 	 m_win_buf; 
	VertexBuffer 	 m_text_win_buf;

	// Enviroment
	FileSystem m_fs;

public:
    Window(int width, int height, char const * title);
    ~Window();

    Window(Window const &)             = delete;
    Window & operator=(Window const &) = delete;

    bool isFullscreen() const { return m_is_fullscreen; }

    void createWindow();
    void initScene();
    void fullscreen(bool is_fullscreen);
    void run();

    // keys
    void key_f1();
};

#endif   // WINDOW_H
