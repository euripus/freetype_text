#include "button.h"

Button::Button(std::string name, UIWindow & owner)
    : Widget(owner),
      m_caption(std::move(name))
{}

void Button::update(float time, bool check_cursor) {}

void Button::draw() {}

void Button::move(glm::vec2 const & new_origin) {}
