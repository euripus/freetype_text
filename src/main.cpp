#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>

#include <iostream>

#include "src/gui/uiimagemanager.h"
#include "src/input/input.h"
#include "gui/fontmanager.h"
#include "gui/imagedata.h"
#include "gui/ui.h"
#include "gui/text_box.h"
#include "vertex_buffer.h"

constexpr char const *  WINDOWTITLE = "GLFW Frame Application";
constexpr char const *  TEXNAME     = "./data/base.tga";
constexpr char const *  TEXTSAMPLE  = "Καρε ανα δευτερολεπτο: %d";
constexpr std::uint32_t WINDOWHEIGT = 600;
constexpr std::uint32_t WINDOWWIDTH = 800;

GLFWwindow * g_window = nullptr;

bool g_wire = false;
// Fps counter
double       g_last_time  = 0.0;
unsigned int g_num_frames = 0;
unsigned int g_num_FPS    = 0;

bool                running        = true;
bool                is_full_screen = false;
GLFWvidmode const * cur_mode;
GLfloat             rty = 0.0f;
GLfloat             rtx = 0.0f;

Input g_input_state;
UI    g_ui;

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

FontManager  fm;
GLuint       tex_base;
VertexBuffer pyramid_buf(VertexBuffer::pos_tex_norm);
VertexBuffer text_buf(VertexBuffer::pos_tex);

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
                running = false;
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
                is_full_screen = !is_full_screen;

                KillWindow();

                if(is_full_screen)
                {
                    width  = cur_mode->width;
                    height = cur_mode->height;
                }
                else
                {
                    width  = WINDOWWIDTH;
                    height = WINDOWHEIGT;
                }
                if(!CreateGLFWWindow(width, height, is_full_screen))
                {
                    std::cerr << "error!" << std::endl;
                    running = false;
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

            GLint  internal_format = (image.type == tex::ImageData::PixelType::pt_rgb) ? GL_RGB : GL_RGB4;
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
    glClearColor(0.0f, 0.1f, 0.4f, 0.5f);
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

    fm.parseFontsRes("./data/ui_res.json");

    // fm.getAtlas().writeAtlasToTGA(std::string("./data/atlas.tga"));
    fm.getAtlas().UploadTexture();

    return LoadTexture();
}

bool CreateGLFWWindow(int width, int height, bool fullscreenflag)
{
    // glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 1);
    // glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 5);
    /*glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/
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
    static char buffer[100];
    if(glfwGetTime() - g_last_time > 1.0)
    {
        g_last_time  = glfwGetTime();
        g_num_FPS    = g_num_frames;
        g_num_frames = 0;

        text_buf.clear();
        auto & tf = *fm.getFont("damase", 24);

        std::sprintf(buffer, TEXTSAMPLE, g_num_FPS);
        glm::vec2 pen(10, 10);
        auto      mt = MarkupText(tf, MarkupText::LineType::UNDERLINE);
        mt.addText(text_buf, buffer, pen);

        pen = glm::vec2(10, 10 + tf.getHeight() + tf.getLineGap());
        auto key_desc = std::string("Последняя клавиша: ") + KeyDescription(g_input_state.getKeyPressed());
        tf.addText(text_buf, key_desc.c_str(), pen);

        pen             = glm::vec2(10, 10 + 2.0f * (tf.getHeight() + tf.getLineGap()));
        auto cursor_pos = g_input_state.getMousePosition();
        std::sprintf(buffer, "Cursor pos: %d, %d", cursor_pos.x, cursor_pos.y);
        tf.addText(text_buf, buffer, pen);

        text_buf.upload();
    }
    g_num_frames++;

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
    glOrtho(0.0, g_ui.m_screen_size.x, 0, g_ui.m_screen_size.y, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    fm.getAtlas().BindTexture();

    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
    text_buf.drawBuffer();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void KillWindow(void)
{
    glDeleteTextures(1, &tex_base);
    fm.getAtlas().DeleteTexture();

    glfwDestroyWindow(g_window);
    g_window = nullptr;
}

static void error_callback(int error, char const * description)
{
    std::cout << error << ": " << description << std::endl;
}

static void print_widget_size(Widget const * widget, int32_t level = 0)
{
    if(!(widget->getType() == ElementType::VerticalLayoutee
         || widget->getType() == ElementType::HorizontalLayoutee))
    {
        for(int32_t i = 0; i < level - 1; ++i)
            std::cout << "\t";
        std::cout << "pos: " << widget->pos().x << " " << widget->pos().y << "  ";
        std::cout << "size: " << widget->size().x << " " << widget->size().y << std::endl;
    }

    if(uint32_t num_ch = widget->getNumChildren(); num_ch > 0)
    {
        for(uint32_t i = 0; i < num_ch; ++i)
        {
            print_widget_size(widget->getChild(i), level + 1);
        }
    }
}

int main()
{
    g_ui.parseUIResources("./data/ui_res.json");
    auto win = g_ui.loadWindow("./data/test_win.json");

    if(auto * text_box = win->getWidgetFromID<TextBox>("text_window"); text_box != nullptr)
        text_box->setText("New text in widget! Veeeeeeeeeeeeeeeeeeeeeeerrrrrrrrrryyyyyyyyy "
                          "loooooooooonnnnnnnnnnggggggggggggg!");

    win->show();
    print_widget_size(g_ui.m_layers[0].front()->getRootWidget());

    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n" << std::endl;
        return 0;
    }

    cur_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if(!CreateGLFWWindow(WINDOWWIDTH, WINDOWHEIGT, is_full_screen))
        return 0;

    // auto device = glGetString(GL_VENDOR);
    // auto renderer = glGetString(GL_RENDERER);

    while(!glfwWindowShouldClose(g_window) && running)
    {
        g_ui.update(glfwGetTime());
        DrawScene();

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    KillWindow();
    glfwTerminate();

    return 0;
}
