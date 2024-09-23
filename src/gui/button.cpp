#include "button.h"
#include "text_fitter.h"
#include "ui.h"

Button::Button(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner),
      m_caption(std::move(desc.static_text))
{
    // Trim text to button size
    auto lines = TextFitter::AdjustTextToSize(*m_font, m_rect.m_size, false, m_caption);
    m_caption  = lines[0];
}

void Button::subClassUdate(float time, bool check_cursor)
{
    if(check_cursor)
    {
        auto const & inp     = m_owner.getOwner().m_input;
        glm::vec2    cur_pos = inp.getMousePosition();

        Rect2D widget_area{m_pos, m_rect.m_size};
        if(widget_area.contains(cur_pos))
        {
            if(inp.isMouseButtonPressed(MouseButton::Left))
            {
                m_state = ButtonState::clicked;

                if(m_click_callback)
                    m_owner.addCallBack(m_click_callback);
            }
            else
            {
                if(m_state == ButtonState::clicked)
                {
                    m_state = ButtonState::unclicked;
                }
            }
        }
        else
        {
            if(m_state == ButtonState::clicked)
            {
                m_state = ButtonState::unclicked;
            }
        }

        m_region_ptr = getRegionFromState(m_state);
    }
}

RegionDataOfUITexture const * Button::getRegionFromState(ButtonState state) const
{
    static constexpr char const * sid_button_clicked   = "button_clicked";
    static constexpr char const * sid_button_unclicked = "button_unclicked";
    static constexpr char const * sid_button_disabled  = "button_disabled";

    if(!m_owner.isImageGroupExist())
        return nullptr;

    RegionDataOfUITexture const * result = nullptr;
    auto const &                  images = m_owner.getImageGroup();
    switch(state)
    {
        case ButtonState::clicked:
        {
            result = images.getImageRegion(sid_button_clicked);
            break;
        }
        case ButtonState::unclicked:
        {
            result = images.getImageRegion(sid_button_unclicked);
            break;
        }
        case ButtonState::disabled:
        {
            result = images.getImageRegion(sid_button_disabled);
            break;
        }
    }

    return result;
}

void Button::subClassDraw(VertexBuffer & background, VertexBuffer & text) const
{
    // draw text
    float const line_height = m_font->getSize();
    float const line_width  = m_font->getTextSize(m_caption.c_str()).x;

    glm::vec2 pen_pos(0.f, 0.f);
    pen_pos.y = m_pos.y + m_rect.height()
                - (line_height + m_fields.w
                   + (m_rect.height() - (m_fields.z + m_fields.w + line_height))
                         / 2.f);   // vertically align to the center only
    pen_pos.x = getHorizontalOffset();

    m_font->addText(text, m_caption.c_str(), pen_pos);
}
