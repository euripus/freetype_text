#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <memory>
#include <stdio.h>
#undef __STRICT_ANSI__

#include "Shader.h"
#include "TexFont.h"
#include "TextRender.h"
#include "Tga.h"
#include "VertexBuffer.h"

#define M_PI 3.14159265358979323846

constexpr char const *  WINDOWTITLE = "GLFW Frame Application";
constexpr char const *  TEXNAME     = "base.tga";
constexpr std::uint32_t WINDOWHEIGT = 600;
constexpr std::uint32_t WINDOWWIDTH = 800;

GLFWwindow * g_window = NULL;

bool g_wire = false;
// Fps counter
double       g_LastTime  = 0;
unsigned int g_numFrames = 0;
unsigned int g_numFPS    = 0;

bool                running      = true;
bool                isFullScreen = false;
GLFWvidmode const * curMode;
GLfloat             rty = 0.0f;
GLfloat             rtx = 0.0f;
unsigned int        g_width, g_height;

GLfloat pyrVert[] = {
    1.41421,  -1,        0,        0,         -1,       0,         0.5,      0.8,       0,        -1,
    1.41421,  0,         -1,       0,         0.2,      0.5,       -1.41421, -1,        0,        0,
    -1,       0,         0.5,      0.2,       0,        -1,        -1.41421, 0,         -1,       0,
    0.8,      0.5,       1.41421,  -1,        0,        0.632456,  0.447214, 0.632456,  0.5,      0.8,
    0,        1,         0,        0.632456,  0.447214, 0.632456,  0,        1,         0,        -1,
    1.41421,  0.632456,  0.447214, 0.632456,  0.2,      0.5,       0,        -1,        1.41421,  -0.632456,
    0.447214, 0.632456,  0.2,      0.5,       0,        1,         0,        -0.632456, 0.447214, 0.632456,
    0,        0,         -1.41421, -1,        0,        -0.632456, 0.447214, 0.632456,  0.5,      0.2,
    -1.41421, -1,        0,        -0.632456, 0.447214, -0.632456, 0.5,      0.2,       0,        1,
    0,        -0.632456, 0.447214, -0.632456, 1,        0,         0,        -1,        -1.41421, -0.632456,
    0.447214, -0.632456, 0.8,      0.5,       0,        1,         0,        0.632456,  0.447214, -0.632456,
    1,        1,         1.41421,  -1,        0,        0.632456,  0.447214, -0.632456, 0.5,      0.8,
    0,        -1,        -1.41421, 0.632456,  0.447214, -0.632456, 0.8,      0.5};

GLuint pyrIndex[] = {13, 14, 15, 7, 8, 9, 4, 5, 6, 10, 11, 12, 0, 1, 2, 0, 2, 3};

// TexFont *    tf;
std::unique_ptr<TexFont> tf;
Shader                   shd, shdTxt;
GLuint                   texBase;
VertexBuffer             pyramidBuf("Pos:3,Norm:3,Tex:2");
VertexBuffer             textBuf("Pos:3,Tex:2");

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
                    isFullScreen = !isFullScreen;

                    KillWindow();

                    if(isFullScreen)
                    {
                        width  = curMode->width;
                        height = curMode->height;
                    }
                    else
                    {
                        width  = WINDOWWIDTH;
                        height = WINDOWHEIGT;
                    }
                    if(!CreateGLFWWindow(width, height, isFullScreen))
                    {
                        printf("error!");
                        running = false;
                    }
                }
                break;
            }
    }
}

void WindowSizeCallback(GLFWwindow * win, int width, int height)
{
    GLfloat xmin, xmax, ymin, ymax, aspect;

    height = height > 0 ? height : 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    aspect = (GLfloat)width / (GLfloat)height;
    ymax   = 0.1f * tan(65.0f * M_PI / 360.0f);
    ymin   = -ymax;
    xmin   = ymin * aspect;
    xmax   = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    g_width  = width;
    g_height = height;
}

void MouseButtonCallback(GLFWwindow * win, int button, int action, int mods) {}

void MousePositionCallback(GLFWwindow * win, double xpos, double ypos) {}

void MouseWheelCallback(GLFWwindow * win, double xoffset, double yoffset) {}

bool LoadTexture()
{
    Texture tex;

    if(LoadTGA(&tex, TEXNAME))
    {
        glGenTextures(1, &texBase);

        glBindTexture(GL_TEXTURE_2D, texBase);

        glTexImage2D(GL_TEXTURE_2D, 0, tex.bpp / 8, tex.width, tex.height, 0, tex.type, GL_UNSIGNED_BYTE,
                     tex.imageData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        free(tex.imageData);

        return true;
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

    pyramidBuf.VertexBufferPushBack(pyrVert, sizeof(pyrVert) / pyramidBuf.GetNumVertComponents(), pyrIndex,
                                    sizeof(pyrIndex) / sizeof(GLuint));
    pyramidBuf.VertexBufferUpload();
    pyramidBuf.InitAttribLocation();

    shd.Init("vert.glsl", "frag.glsl");

    std::string str2(
        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
        "АБВГДЕЖЗИКЛМНОПРСТУФХЦЧШЩЫЬЭЮЯабвгдежзиклмнопрстуфхцчшщъыьэюя");

    tf = std::make_unique<TexFont>(24, std::string("Liberation.ttf"));
    tf->textureFontCacheGlyphs(str2.c_str());
    tf->getAtlas().WriteAtlasToTGA(std::string("atlas.tga"));
    tf->getAtlas().UploadTexture();
    shdTxt.Init("vertTxt.glsl", "fragTxt.glsl");
    glm::vec2 pos(10, 40);
    AddText(textBuf, *tf, "FPS: 60", pos);
    textBuf.VertexBufferUpload();
    textBuf.InitAttribLocation();

    return LoadTexture();
}

bool CreateGLFWWindow(int width, int height, bool fullscreenflag)
{
    /*glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor * mon = fullscreenflag ? glfwGetPrimaryMonitor() : NULL;
    g_window          = glfwCreateWindow(width, height, WINDOWTITLE, mon, NULL);
    if(g_window == NULL)
    {
        printf("error!");
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
    if(glfwGetTime() - g_LastTime > 1.0)
    {
        g_LastTime  = glfwGetTime();
        g_numFPS    = g_numFrames;
        g_numFrames = 0;

        textBuf.Clear();
        std::sprintf(buffer, "Кадров в секунду: %d", g_numFPS);
        glm::vec2 pen(10, 40);
        AddText(textBuf, *tf, buffer, pen);
        textBuf.VertexBufferUpload();
        textBuf.InitAttribLocation();
    }
    g_numFrames++;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(rty, 0.0f, 1.0f, 0.0f);
    glRotatef(rtx, 1.0f, 0.0f, 0.0f);

    shd.Bind();

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(shd.Id(), "baseMap"), 0);
    glBindTexture(GL_TEXTURE_2D, texBase);

    pyramidBuf.DrawBuffer();
    shd.Unbind();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, g_width, 0, g_height, -1.0, 1.0);   // Левая, правая, нижняя, верхняя, ближняя, дальняя
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    shdTxt.Bind();
    glUniform1i(glGetUniformLocation(shd.Id(), "baseMap"), 0);
    tf->getAtlas().BindTexture();

    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
    textBuf.DrawBuffer();
    shdTxt.Unbind();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void KillWindow(void)
{
    pyramidBuf.DeleteGPUBuffers();
    shd.DeInit();
    glDeleteTextures(1, &texBase);
    tf->getAtlas().DeleteTexture();
    tf->getAtlas().clear();
    shdTxt.DeInit();

    glfwDestroyWindow(g_window);
    g_window = NULL;
}

static void error_callback(int error, char const * description)
{
    printf("%d: %s", error, description);
}

int main()
{
    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 0;
    }

    curMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if(!CreateGLFWWindow(WINDOWWIDTH, WINDOWHEIGT, isFullScreen))
        return 0;

    while(!glfwWindowShouldClose(g_window) && running)
    {
        DrawScene();

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    KillWindow();
    glfwTerminate();

    return 0;
}
