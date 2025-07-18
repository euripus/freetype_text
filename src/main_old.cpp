#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>

#include <iostream>

#include "src/input/input.h"
#include "gui/ui.h"
#include "gui/text_box.h"
#include "gui/button.h"
#include "vertex_buffer.h"

#include "./fs/file_system.h"

constexpr char const *  WINDOWTITLE = "GLFW Frame Application";
constexpr char const *  TEXNAME     = "base.tga";
constexpr std::uint32_t WINDOWHEIGT = 600;
constexpr std::uint32_t WINDOWWIDTH = 800;

GLFWwindow * g_window = nullptr;

FileSystem g_fs("./data");

bool         g_wire    = false;
unsigned int g_num_fps = 0;   // Fps counter

bool                g_running        = true;
bool                g_is_full_screen = false;
GLFWvidmode const * g_monitor_mode;
GLfloat             rty = 0.0f;
GLfloat             rtx = 0.0f;

Input        g_input_state;
UI           g_ui(g_input_state, g_fs);
VertexBuffer win_buf(VertexBuffer::pos_tex), text_win_buf(VertexBuffer::pos_tex),
    pyramid_buf(VertexBuffer::pos_tex_norm);
GLuint tex_base;

/*-----------------------------------------------------------
/
/-----------------------------------------------------------*/
bool InitWindow()
{
    glfwSetWindowTitle(g_window, WINDOWTITLE);

    glfwSetKeyCallback(g_window, KeyFuncCallback);
    glfwSetCursorPosCallback(g_window, MousePositionCallback);
    glfwSetMouseButtonCallback(g_window, MouseButtonCallback);
    glfwSetScrollCallback(g_window, MouseWheelCallback);
    glfwSetWindowSizeCallback(g_window, WindowSizeCallback);

    glShadeModel(GL_SMOOTH);
    glClearColor(ColorMap::navy.r, ColorMap::navy.g, ColorMap::navy.b, ColorMap::navy.a);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    // glDepthFunc( GL_LEQUAL );

    if(!InitExtensions())
    {
        return false;
    }

    pyramid_buf.pushBack(pyr_vert_pos, pyr_vert_tex, pyr_vert_norm,
                         sizeof(pyr_vert_pos) / (sizeof(float) * 3), pyr_index,
                         sizeof(pyr_index) / sizeof(GLuint));
    pyramid_buf.upload();

    g_ui.getUIImageAtlas().UploadTexture();
    g_ui.getFontImageAtlas().UploadTexture();

    win_buf.upload();
    text_win_buf.upload();

    return LoadTexture();
}

bool CreateGLFWWindow(int32_t width, int32_t height, bool fullscreenflag)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor * mon = fullscreenflag ? glfwGetPrimaryMonitor() : NULL;
    g_window          = glfwCreateWindow(width, height, WINDOWTITLE, mon, NULL);
    if(g_window == nullptr)
    {
        std::cerr << "error!\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(g_window);

    WindowSizeCallback(g_window, width, height);

    return InitWindow();
}

void DrawScene(void)
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

    g_ui.draw(win_buf, text_win_buf);
    VertexBufferBinder win_buf_bind(win_buf), text_win_buf_bind(text_win_buf);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(rty, 0.0f, 1.0f, 0.0f);
    glRotatef(rtx, 1.0f, 0.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_base);

    pyramid_buf.drawBuffer();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, g_ui.getScreenSize().x, 0, g_ui.getScreenSize().y, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    g_ui.getUIImageAtlas().BindTexture();
    win_buf.drawBuffer();

    g_ui.getFontImageAtlas().BindTexture();
    glColor4fv(glm::value_ptr(g_ui.getFontColor()));
    text_win_buf.drawBuffer();
    glColor4fv(glm::value_ptr(ColorMap::white));

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void KillWindow(void)
{
    glDeleteTextures(1, &tex_base);
    g_ui.getFontImageAtlas().DeleteTexture();
    g_ui.getUIImageAtlas().DeleteTexture();

    glfwDestroyWindow(g_window);
    g_window = nullptr;
}
