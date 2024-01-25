#include "text_box.h"
#include "window.h"
#include "text_fitter.h"

TextBox::TextBox(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_text(desc.static_text)
{
    adjustTextToLines();
}

void TextBox::update(float time, bool check_cursor) {}

void TextBox::draw(VertexBuffer & vb) {}

void TextBox::move(glm::vec2 const & new_origin)
{
    Widget::move(new_origin);

    if(!m_formated)
        adjustTextToLines();
}

void TextBox::setText(std::string const & new_text)
{
    m_text     = new_text;
    m_formated = false;

    adjustTextToLines();
}

void TextBox::adjustTextToLines()
{
    m_lines = TextFitter::AdjustTextToRect(*m_font, m_rect, m_scale, m_text);

    float text_height = m_lines.size() * (m_font->getHeight() + m_font->getLineGap());
    float text_width  = TextFitter::MaxStringWidthInLines(*m_font, m_lines);
    if(m_scale == SizePolicy::fixed_width)
        text_width = glm::max(text_width, m_size_hint.x);
    else if(m_scale == SizePolicy::fixed_height)
        text_height = glm::max(text_height, m_size_hint.y);

    m_rect.m_extent = glm::vec2(text_width, text_height);
    m_formated      = true;

    m_owner.childResized();   // resize text area message
}
