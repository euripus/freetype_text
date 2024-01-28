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

void TextBox::update(float time, bool check_cursor) {}

void TextBox::subClassDraw(VertexBuffer & background, VertexBuffer & text) 
{
    if(m_font == nullptr || !m_formated)
        return;

    //draw text
    float const line_height = m_font->getHeight() + m_font->getLineGap();
    glm::vec2 pen_pos(0.f, 0.f);    
    pen_pos.y = m_pos.y + m_rect.height() - line_height;

    for(auto const & line : m_lines)
    {
        switch(m_text_horizontal_align)
        {
            case Align::left:
            case Align::top:          // horizontal align only
            case Align::bottom:
            {
                pen_pos.x = m_pos.x;

                break;
            }
            case Align::center:
            {
                float const line_width = m_font->getTextSize(line.c_str()).x;
                pen_pos.x = m_pos.x + (m_rect.width() - line_width)/2.f;

                break;
            }
            case Align::right:
            {
                float const line_width = m_font->getTextSize(line.c_str()).x;
                pen_pos.x = m_pos.x + (m_rect.width() - line_width);

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
