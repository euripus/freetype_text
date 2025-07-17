#include "text_box.h"
#include "../vertex_buffer.h"
#include "window.h"
#include "uiconfigloader.h"
#include "text_fitter.h"

TextBox::TextBox(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_text(desc.static_text)
{
    adjustTextToLines();
}

void TextBox::subClassDraw(VertexBuffer & background, VertexBuffer & text) const
{
    if(!m_formated)
        return;

    // draw text
    float const line_height = m_font->getHeight() + m_font->getLineGap();
    float       y           = m_pos.y + m_rect.height() - (line_height + m_fields.w);

    for(auto const & line: m_lines)
    {
        glm::vec2 text_pos;
        text_pos.x = getHorizontalOffset(line);
        text_pos.y = y + getVerticalOffset();

        m_font->addText(text, line.c_str(), text_pos);
        y -= line_height;
    }
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
}
