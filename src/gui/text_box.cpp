#include "text_box.h"
#include "window.h"
#include "text_fitter.h"

TextBox::TextBox(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_text(desc.static_text),
      m_text_horizontal_align(desc.text_hor)
{
    adjustTextToLines();
}

void TextBox::subClassDraw(VertexBuffer & background, VertexBuffer & text) const
{
    if(m_font == nullptr || !m_formated)
        return;

    // draw text
    float const line_height = m_font->getHeight() + m_font->getLineGap();
    glm::vec2   pen_pos(0.f, 0.f);
    pen_pos.y = m_pos.y + m_rect.height() - (line_height + m_fields.w);

    for(auto const & line: m_lines)
    {
        switch(m_text_horizontal_align)
        {
            case Align::left:
            case Align::top:   // horizontal align only
            case Align::bottom:
            {
                pen_pos.x = m_pos.x + m_fields.x;

                break;
            }
            case Align::center:
            {
                float const line_width = m_font->getTextSize(line.c_str()).x;
                pen_pos.x              = m_pos.x + (m_rect.width() - line_width) / 2.f;

                break;
            }
            case Align::right:
            {
                float const line_width = m_font->getTextSize(line.c_str()).x;
                float const delta      = glm::max(m_fields.x, (m_rect.width() - line_width - m_fields.y));
                pen_pos.x              = m_pos.x + delta;

                break;
            }
        }

        m_font->addText(text, line.c_str(), pen_pos);
        pen_pos.y -= line_height;
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
    Rect2D fit_rect{m_rect.m_pos, glm::vec2(m_rect.width() - m_fields.x - m_fields.y,
                                            m_rect.height() - m_fields.z - m_fields.w)};
    bool   stretch = (m_stretch > 0.f);
    m_lines        = TextFitter::AdjustTextToRect(*m_font, fit_rect, stretch, m_text);

    float text_height = m_lines.size() * (m_font->getHeight() + m_font->getLineGap());
    float text_width  = TextFitter::MaxStringWidthInLines(*m_font, m_lines);
    text_width        = glm::max(text_width, m_rect.width());
    text_height       = glm::max(text_height, m_rect.height());

    m_rect.m_size = glm::vec2(text_width + m_fields.x + m_fields.y, text_height + m_fields.z + m_fields.w);
    m_formated    = true;

    sizeUpdated();
}
