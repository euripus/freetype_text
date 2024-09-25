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

constexpr char const *  WINDOWTITLE = "GLFW Frame Application";
constexpr char const *  TEXNAME     = "./data/base.tga";
constexpr std::uint32_t WINDOWHEIGT = 600;
constexpr std::uint32_t WINDOWWIDTH = 800;

GLFWwindow * g_window = nullptr;

bool         g_wire    = false;
unsigned int g_num_fps = 0;   // Fps counter

bool                g_running        = true;
bool                g_is_full_screen = false;
GLFWvidmode const * g_monitor_mode;
GLfloat             rty = 0.0f;
GLfloat             rtx = 0.0f;

// clang-format off
GLfloat pyr_vert[] = {
    1.41421,  -1,        0,        0,         -1,       0,         0.5,      0.8,
    0,        -1,        1.41421,  0,         -1,       0,         0.2,      0.5,
    -1.41421, -1,        0,        0,         -1,       0,         0.5,      0.2,
    0,        -1,        -1.41421, 0,         -1,       0,         0.8,      0.5,
    1.41421,  -1,        0,        0.632456,  0.447214, 0.632456,  0.5,      0.8,
    0,         1,        0,        0.632456,  0.447214, 0.632456,  0,        1,
    0,        -1,        1.41421,  0.632456,  0.447214, 0.632456,  0.2,      0.5,
    0,        -1,        1.41421,  -0.632456, 0.447214, 0.632456,  0.2,      0.5,
    0,        1,         0,        -0.632456, 0.447214, 0.632456,  0,        0,
    -1.41421, -1,        0,        -0.632456, 0.447214, 0.632456,  0.5,      0.2,
    -1.41421, -1,        0,        -0.632456, 0.447214, -0.632456, 0.5,      0.2,
    0,        1,         0,        -0.632456, 0.447214, -0.632456, 1,        0,
    0,        -1,        -1.41421, -0.632456, 0.447214, -0.632456, 0.8,      0.5,
    0,        1,         0,        0.632456,  0.447214, -0.632456, 1,        1,
    1.41421,  -1,        0,        0.632456,  0.447214, -0.632456, 0.5,      0.8,
    0,        -1,        -1.41421, 0.632456,  0.447214, -0.632456, 0.8,      0.5
};
// clang-format on

GLfloat pyr_vert_norm[] = {0,         -1,        0,         0,         -1,        0,         0,
                           -1,        0,         0,         -1,        0,         0.632456,  0.447214,
                           0.632456,  0.632456,  0.447214,  0.632456,  0.632456,  0.447214,  0.632456,
                           -0.632456, 0.447214,  0.632456,  -0.632456, 0.447214,  0.632456,  -0.632456,
                           0.447214,  0.632456,  -0.632456, 0.447214,  -0.632456, -0.632456, 0.447214,
                           -0.632456, -0.632456, 0.447214,  -0.632456, 0.632456,  0.447214,  -0.632456,
                           0.632456,  0.447214,  -0.632456, 0.632456,  0.447214,  -0.632456};

GLfloat pyr_vert_pos[] = {
    1.41421, -1, 0,        0, -1, 1.41421, -1.41421, -1, 0, 0,        -1, -1.41421, 1.41421,  -1, 0, 0, 1, 0,
    0,       -1, 1.41421,  0, -1, 1.41421, 0,        1,  0, -1.41421, -1, 0,        -1.41421, -1, 0, 0, 1, 0,
    0,       -1, -1.41421, 0, 1,  0,       1.41421,  -1, 0, 0,        -1, -1.41421};

GLfloat pyr_vert_tex[] = {0.5, 0.8, 0.2, 0.5, 0.5, 0.2, 0.8, 0.5, 0.5, 0.8, 0, 1, 0.2, 0.5, 0.2, 0.5,
                          0,   0,   0.5, 0.2, 0.5, 0.2, 1,   0,   0.8, 0.5, 1, 1, 0.5, 0.8, 0.8, 0.5};

GLuint pyr_index[] = {13, 14, 15, 7, 8, 9, 4, 5, 6, 10, 11, 12, 0, 1, 2, 0, 2, 3};

Input        g_input_state;
UI           g_ui(g_input_state);
VertexBuffer win_buf(VertexBuffer::pos_tex), text_win_buf(VertexBuffer::pos_tex),
    pyramid_buf(VertexBuffer::pos_tex_norm);
GLuint tex_base;

/*-----------------------------------------------------------
/
/-----------------------------------------------------------*/
bool InitExtensions()
{
    GLenum err = glewInit();
    if(GLEW_OK != err)
    {
        return false;
    }

    // Vertex buffer object
    if(!GLEW_ARB_vertex_buffer_object)
    {
        return false;
    }

    return true;
}
/*-----------------------------------------------------------
/
/-----------------------------------------------------------*/

bool CreateGLFWWindow(int width, int height, bool fullscreenflag);
void KillWindow(void);

void KeyFuncCallback(GLFWwindow * win, int key, int scancode, int action, int mods)
{
    int width, height;

    switch(key)
    {
        case GLFW_KEY_ESCAPE:
        {
            if(action == GLFW_PRESS)
            {
                g_running = false;
            }
            break;
        }
        case GLFW_KEY_LEFT:
        {
            if(action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                rty -= 5.0f;
            }
            break;
        }
        case GLFW_KEY_RIGHT:
        {
            if(action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                rty += 5.0f;
            }
            break;
        }
        case GLFW_KEY_DOWN:
        {
            if(action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                rtx -= 5.0f;
            }
            break;
        }
        case GLFW_KEY_UP:
        {
            if(action == GLFW_PRESS || action == GLFW_REPEAT)
            {
                rtx += 5.0f;
            }
            break;
        }
        case 'W':
        {
            if(action == GLFW_PRESS)
            {
                g_wire = (!g_wire);
                if(g_wire)
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                }
                else
                {
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
            }
            break;
        }
        case GLFW_KEY_F1:
        {
            if(action == GLFW_PRESS)
            {
                g_is_full_screen = !g_is_full_screen;

                KillWindow();

                if(g_is_full_screen)
                {
                    width  = g_monitor_mode->width;
                    height = g_monitor_mode->height;
                }
                else
                {
                    width  = WINDOWWIDTH;
                    height = WINDOWHEIGT;
                }
                if(!CreateGLFWWindow(width, height, g_is_full_screen))
                {
                    std::cerr << "error!" << std::endl;
                    g_running = false;
                }
            }
            break;
        }
    }

    bool pressed = (action != GLFW_RELEASE);
    g_input_state.keyEvent(MapKeyCode(key), pressed);
}

void WindowSizeCallback(GLFWwindow * win, int width, int height)
{
    GLfloat xmin, xmax, ymin, ymax, aspect;

    height = height > 0 ? height : 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    aspect = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);
    ymax   = 0.1f * glm::tan(65.0f * glm::pi<float>() / 360.0f);
    ymin   = -ymax;
    xmin   = ymin * aspect;
    xmax   = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    g_input_state.resize(width, height);
    g_ui.resize(width, height);
}

void MouseButtonCallback(GLFWwindow * win, int32_t button, int32_t action, int32_t mods)
{
    MouseButton button_id = MouseButton::ButtonsCount;

    if(button == GLFW_MOUSE_BUTTON_LEFT)
        button_id = MouseButton::Left;
    else if(button == GLFW_MOUSE_BUTTON_MIDDLE)
        button_id = MouseButton::Middle;
    else if(button == GLFW_MOUSE_BUTTON_RIGHT)
        button_id = MouseButton::Right;

    bool pressed = (action != GLFW_RELEASE);

    g_input_state.buttonEvent(button_id, pressed);
}

void MousePositionCallback(GLFWwindow * win, double xpos, double ypos)
{
    g_input_state.mousePos(static_cast<int32_t>(xpos), static_cast<int32_t>(ypos));
}

void MouseWheelCallback(GLFWwindow * win, double xoffset, double yoffset)
{
    g_input_state.mouseWhell(static_cast<int32_t>(yoffset));
}

bool LoadTexture()
{
    {
        tex::ImageData image;

        if(tex::ReadTGA(TEXNAME, image))
        {
            glGenTextures(1, &tex_base);

            glBindTexture(GL_TEXTURE_2D, tex_base);

            GLint  internal_format = (image.type == tex::ImageData::PixelType::pt_rgb) ? GL_RGB : GL_RGBA;
            GLenum format          = (image.type == tex::ImageData::PixelType::pt_rgb) ? GL_RGB : GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image.width, image.height, 0, format,
                         GL_UNSIGNED_BYTE, image.data.get());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);

            return true;
        }
    }

    return false;
}

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

void SetUIData(UIWindow * win)
{
    std::string const fps   = std::to_string(g_num_fps);
    std::string const key   = KeyDescription(g_input_state.getKeyPressed());
    std::string const cur_x = std::to_string(g_input_state.getMousePosition().x);
    std::string const cur_y = std::to_string(g_input_state.getMousePosition().y);

    if(auto * text_box = win->getWidgetFromID<TextBox>("fps_num"); text_box != nullptr)
        text_box->setText(fps);

    if(auto * text_box = win->getWidgetFromID<TextBox>("key_num"); text_box != nullptr)
        text_box->setText(key);

    if(auto * text_box = win->getWidgetFromID<TextBox>("pos_x"); text_box != nullptr)
        text_box->setText(cur_x);

    if(auto * text_box = win->getWidgetFromID<TextBox>("pos_y"); text_box != nullptr)
        text_box->setText(cur_y);
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

static void error_callback(int error, char const * description)
{
    std::cout << error << ": " << description << std::endl;
}

int main()
{
    g_ui.parseUIResources("./data/ui/jsons/ui_res.json");
    UIWindow * win = g_ui.loadWindow("./data/ui/jsons/vert_win.json");

    if(auto * button = win->getWidgetFromID<Button>("button_ok"); button != nullptr)
    {
        button->setCallback([] {
            int32_t width{0}, height{0};
            g_is_full_screen = !g_is_full_screen;

            KillWindow();

            if(g_is_full_screen)
            {
                width  = g_monitor_mode->width;
                height = g_monitor_mode->height;
            }
            else
            {
                width  = WINDOWWIDTH;
                height = WINDOWHEIGT;
            }
            if(!CreateGLFWWindow(width, height, g_is_full_screen))
            {
                std::cerr << "error!" << std::endl;
                g_running = false;
            }
        });
    }
    if(auto * button = win->getWidgetFromID<Button>("button_close"); button != nullptr)
    {
        button->setCallback([] { g_running = false; });
    }

    win->show();
    win->move({10.f, 10.f});

    // g_ui.getFontImageAtlas().writeAtlasToTGA(std::string("./data/atlas.tga"));
    // g_ui.getUIImageAtlas().writeAtlasToTGA(std::string("./data/atlas_ui.tga"));

    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return 0;
    }

    g_monitor_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if(!CreateGLFWWindow(WINDOWWIDTH, WINDOWHEIGT, g_is_full_screen))
        return 0;

    while(!glfwWindowShouldClose(g_window) && g_running)
    {
        SetUIData(win);
        g_ui.update(glfwGetTime());
        DrawScene();

        glfwSwapBuffers(g_window);

        g_input_state.clearEventQueues();
        glfwPollEvents();
    }

    KillWindow();
    glfwTerminate();

    return 0;
}
