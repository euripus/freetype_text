#include "button.h"
#include "text_fitter.h"

Button::Button(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_caption(std::move(desc.static_text))
{
    // Trim text to button size
    auto lines = TextFitter::AdjustTextToRect(*m_font, m_rect, SizePolicy::trim, m_caption);
    m_caption  = lines[0];
}

void Button::update(float time, bool check_cursor) {}

void Button::draw(VertexBuffer & vb) {}

void Button::move(glm::vec2 const & new_origin) {}
