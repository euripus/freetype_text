#include "button.h"
#include "text_fitter.h"

Button::Button(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_caption(std::move(desc.static_text)),
      m_text_horizontal_align(desc.text_hor)
{
    // Trim text to button size
    auto lines = TextFitter::AdjustTextToRect(*m_font, m_rect, SizePolicy::trim, m_caption);
    m_caption  = lines[0];
}

void Button::update(float time, bool check_cursor) {}

RegionDataOfUITexture const * Button::getRegionFromState(ButtonState state) const
{
    static constexpr char const * sid_button_clicked      = "button_clicked";
    static constexpr char const * sid_button_unclicked      = "button_unclicked";
    static constexpr char const * sid_button_disabled      = "button_disabled";
    
    RegionDataOfUITexture const * result = nullptr;
    UI const & ui = m_owner.getOwner();
    switch(state)
    {
        case ButtonState::clicked:
        {
            result = ui.getImageRegion(sid_button_clicked);
            break;
        }
        case ButtonState::unclicked:
        {
            result = ui.getImageRegion(sid_button_unclicked);
            break;
        }
        case ButtonState::disabled:
        {
            result = ui.getImageRegion(sid_button_disabled);
            break;
        }
    }

    return result;
}

void Button::subClassDraw(VertexBuffer & background, VertexBuffer & text) 
{
    if(m_font == nullptr)
        return;

    if(!m_formated)
        adjustTextToLines();

    //draw text
    float const line_height = m_font->getHeight() + m_font->getLineGap();
    float const line_width = m_font->getTextSize(m_caption.c_str()).x;
    glm::vec2 pen_pos(0.f, 0.f);    
    pen_pos.y = m_pos.y + m_rect.height() - line_height;

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
            pen_pos.x = m_pos.x + (m_rect.width() - line_width)/2.f;

            break;
        }
        case Align::right:
        {
            pen_pos.x = m_pos.x + (m_rect.width() - line_width);

            break;
        }
    }
    
    m_font->addText(text, m_caption.c_str(), pen_pos);
}
