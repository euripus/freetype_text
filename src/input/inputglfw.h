#ifndef INPUTGLFW_H
#define INPUTGLFW_H

#include "input.h"

class GLFWwindow;

class InputGLFW : public Input
{
public:
    InputGLFW(GLFWwindow * window);
    ~InputGLFW() override = default;
};

#endif   // INPUTGLFW_H
