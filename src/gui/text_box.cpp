#include "text_box.h"
#include "../vertex_buffer.h"
#include "window.h"
#include "text_fitter.h"

TextBox::TextBox(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_text(desc.static_text)
{
    adjustTextToLines();
}

void TextBox::subClassDraw(VertexBuffer & background, VertexBuffer & text) const
{
    // draw text
    float const line_height = m_font->getHeight() + m_font->getLineGap();
    glm::vec2   pen_pos(0.f, 0.f);
    pen_pos.y = m_pos.y + m_rect.height() - (line_height + m_fields.w);
    // glm::vec2 r_size = m_rect.m_size;
    // glm::vec2 r_pos  = m_pos;

    for(auto const & line: m_lines)
    {
        pen_pos.x = getHorizontalOffset();

        m_font->addText(text, line.c_str(), pen_pos);
        pen_pos.y -= line_height;
    }

    // Add2DRectangle(text, r_pos.x, r_pos.y, r_pos.x + r_size.x, r_pos.y + r_size.y, 0.005859375f, 0.015625f,
    // 0.0078125f, 0.017578125f);
}

void TextBox::setText(std::string new_text)
{
    m_text     = std::move(new_text);
    m_formated = false;

    adjustTextToLines();
}

void TextBox::adjustTextToLines()
{
    glm::vec2 fit_size{m_rect.width() - m_fields.x - m_fields.y, m_rect.height() - m_fields.z - m_fields.w};
    m_lines = TextFitter::AdjustTextToSize(*m_font, fit_size, false, m_text);

    m_formated = true;

    sizeUpdated();
}
