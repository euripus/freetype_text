#include "widget.h"
#include "uiwindow.h"
#include "ui.h"
#include <algorithm>
#include <cassert>
#include "uiconfigloader.h"
#include "src/render/vertex_buffer.h"

Widget::Widget(WidgetDesc const & desc, UIWindow & owner)
    : m_owner(owner)
{
    m_min_size              = desc.min_size;
    m_max_size              = desc.max_size;
    m_rect                  = Rect2D(glm::vec2(0.f, 0.f), m_min_size);
    m_id                    = desc.id_name;
    m_region_name           = desc.region_name;
    m_visible               = desc.visible;
    m_horizontal            = desc.horizontal;
    m_vertical              = desc.vertical;
    m_text_horizontal_align = desc.text_hor;
    m_type                  = desc.type;
    m_stretch               = desc.stretch;

    UI & ui = m_owner.getOwner();
    if(auto ptr = ui.m_fonts.getFont(desc.font_name, desc.size); ptr != nullptr)
    {
        m_font = ptr;
    }
    else
    {
        m_font = ui.m_default_font;
    }

    if(!m_region_name.empty() && m_owner.isImageGroupExist())
        m_region_ptr = m_owner.getImageGroup().getImageRegion(m_region_name);
}

void Widget::update(float time, bool check_cursor)
{
    for(auto & ch: m_children)
        ch->update(time, check_cursor);

    subClassUpdate(time, check_cursor);
}

void Widget::draw(VertexBuffer & background, VertexBuffer & text) const
{
    if(m_region_ptr != nullptr && visible())
    {
        glm::vec2 pos = m_pos;
        m_region_ptr->addBlock(background, pos, m_rect.m_size);
    }

    // draw children
    for(auto & ch: m_children)
        ch->draw(background, text);

    if(visible())
        subClassDraw(background, text);
}

void Widget::move(glm::vec2 const & new_origin)
{
    m_pos = m_rect.m_pos + new_origin;

    for(auto & ch: m_children)
        ch->move(new_origin);
}

void Widget::addWidget(std::unique_ptr<Widget> widget)
{
    assert(m_type == ElementType::VerticalLayoutee || m_type == ElementType::HorizontalLayoutee);

    widget->m_parent = this;
    m_children.push_back(std::move(widget));
}

void Widget::removeWidget(Widget * widget)
{
    auto it = std::remove_if(m_children.begin(), m_children.end(),
                             [widget](auto const & ptr) { return widget == ptr.get(); });
    m_children.erase(it, m_children.end());
    // std::erase_if(m_children, [widget](auto & ptr) { return widget == ptr.get();}) c++20
}

bool Widget::isChild(Widget * parent_widget)
{
    Widget const * parent_ptr = parent();
    while(parent_ptr)
    {
        if(parent_ptr == parent_widget)
        {
            return true;
        }
        parent_ptr = parent_ptr->parent();
    }

    return false;
}

Widget * Widget::getWidgetFromIDName(std::string const & id_name)
{
    if(m_id == id_name)
        return this;

    for(auto const & ch: m_children)
    {
        if(auto * ptr = ch->getWidgetFromIDName(id_name); ptr != nullptr)
            return ptr;
    }

    return nullptr;
}

void Widget::sizeUpdated()
{
    m_owner.sizeUpdated();
}

float Widget::getHorizontalOffset(std::string const & line) const
{
    float res = 0.f;
    switch(m_text_horizontal_align)
    {
        case Align::left:
        case Align::top:   // horizontal align only
        case Align::bottom:
        {
            res = m_pos.x + m_fields.x;

            break;
        }
        case Align::center:
        {
            float const line_width = m_font->getTextSize(line.c_str()).x;
            res                    = m_pos.x + (m_rect.width() - line_width) / 2.f;

            break;
        }
        case Align::right:
        {
            float const line_width = m_font->getTextSize(line.c_str()).x;
            float const delta      = glm::max(m_fields.x, (m_rect.width() - line_width - m_fields.y));
            res                    = m_pos.x + delta;

            break;
        }
    }

    return res;
}

float Widget::getVerticalOffset() const
{
    return glm::abs(m_font->getDescender()) + m_font->getLineGap();
}
