#include "widget.h"
#include "window.h"
#include "ui.h"
#include <algorithm>
#include <cassert>
#include "../vertex_buffer.h"

ElementType WidgetDesc::GetElementTypeFromString(std::string_view name)
{
    ElementType type = ElementType::Unknown;

    if(name == "TextBox")
        type = ElementType::TextBox;
    else if(name == "ImageBox")
        type = ElementType::ImageBox;
    else if(name == "Button")
        type = ElementType::Button;
    else if(name == "CheckBox")
        type = ElementType::CheckBox;
    // else if(name == "RadioButton")
    // type = ElementType::RadioButton;
    else if(name == "Slider")
        type = ElementType::Slider;
    else if(name == "ProgressBar")
        type = ElementType::ProgressBar;
    else if(name == "InputBox")
        type = ElementType::InputBox;
    else if(name == "ScrollView")
        type = ElementType::ScrollView;
    else if(name == "VerticalLayoutee")
        type = ElementType::VerticalLayoutee;
    else if(name == "HorizontalLayoutee")
        type = ElementType::HorizontalLayoutee;

    return type;
}

SizePolicy WidgetDesc::GetSizePolicyFromString(std::string_view name)
{
    SizePolicy policy = SizePolicy::none;

    if(name == "scale")
        policy = SizePolicy::scale;
    else if(name == "fixed_width")
        policy = SizePolicy::fixed_width;
    else if(name == "fixed_height")
        policy = SizePolicy::fixed_height;
    else if(name == "trim")
        policy = SizePolicy::trim;

    return policy;
}

Align WidgetDesc::GetAlignFromString(std::string_view name)
{
    Align align = Align::left;

    if(name == "left")
        align = Align::left;
    else if(name == "center")
        align = Align::center;
    else if(name == "right")
        align = Align::right;
    else if(name == "top")
        align = Align::top;
    else if(name == "bottom")
        align = Align::bottom;

    return align;
}

Widget::Widget(WidgetDesc const & desc, UIWindow & owner) : m_owner(owner)
{
    m_size_hint   = desc.size_hint;
    m_rect        = Rect2D(glm::vec2(0.f, 0.f), m_size_hint);
    m_id          = desc.id_name;
    m_region_name = desc.region_name;
    m_visible     = desc.visible;
    m_horizontal  = desc.horizontal;
    m_vertical    = desc.vertical;
    m_scale       = desc.scale;
    m_type        = desc.type;

    UI & ui = m_owner.getOwner();
    if(!desc.font_name.empty())
        m_font = ui.m_fonts.getFont(desc.font_name, desc.size);

    if(!m_region_name.empty() && m_owner.isImageGroupExist())
        m_region_ptr = m_owner.getImageGroup().getImageRegion(m_region_name);
}

void Widget::update(float time, bool check_cursor)
{
    for(auto & ch : m_children)
        ch->update(time, check_cursor);

    subClassUdate(time, check_cursor);
}

void Widget::draw(VertexBuffer & background, VertexBuffer & text) const
{
    if(m_region_ptr != nullptr && visible())
    {
        glm::vec2 pos = m_pos;
        m_region_ptr->addBlock(background, pos, m_rect.m_extent);
    }

    // draw children
    for(auto & ch : m_children)
        ch->draw(background, text);

    if(visible())
        subClassDraw(background, text);
}

void Widget::move(glm::vec2 const & new_origin)
{
    m_pos = m_rect.m_pos + new_origin;

    for(auto & ch : m_children)
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

    for(auto const & ch : m_children)
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
