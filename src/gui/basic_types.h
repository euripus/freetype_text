#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <glm/glm.hpp>
#include <map>
#include "../render/vertex_buffer.h"

enum class ElementType
{
    Empty,
    TextBox,
    ImageBox,   // animated/static image and/or internal rendered frame
    Button,
    // RadioButton,
    CheckBox,
    Slider,
    ProgressBar,
    InputBox,
    ScrollView,
    VerticalLayoutee,
    HorizontalLayoutee,
    Unknown
};

enum class Align
{
    left,
    center,
    right,
    top,
    bottom
};

namespace ColorMap
{
// Standart colors
constexpr glm::vec4 black   = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 silver  = glm::vec4(.75f, .75f, .75f, 1.0f);
constexpr glm::vec4 gray    = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
constexpr glm::vec4 white   = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec4 maroon  = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 red     = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
constexpr glm::vec4 purple  = glm::vec4(0.5f, 0.0f, 0.5f, 1.0f);
constexpr glm::vec4 fuchsia = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 green   = glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
constexpr glm::vec4 lime    = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 olive   = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
constexpr glm::vec4 yellow  = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
constexpr glm::vec4 navy    = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
constexpr glm::vec4 blue    = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
constexpr glm::vec4 teal    = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);
constexpr glm::vec4 aqua    = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);

struct EpsilonLessVec4
{
    glm::vec4 epsilon;

    explicit EpsilonLessVec4(float e = std::numeric_limits<float>::epsilon()) : epsilon(e) {}

    bool operator()(glm::vec4 const & lhs, glm::vec4 const & rhs) const
    {
        return glm::any(glm::greaterThan(rhs - lhs, epsilon));
    }
};

using ColoredTextBuffers = std::map<glm::vec4, VertexBuffer, EpsilonLessVec4>;

VertexBuffer & GetOrCreate(ColoredTextBuffers & buffer, glm::vec4 const & color);
}   // namespace ColorMap

#endif
