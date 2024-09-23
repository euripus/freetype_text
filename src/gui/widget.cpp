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

std::unique_ptr<Widget> Widget::GetWidgetFromJson(boost::json::object const & obj, UIWindow & owner)
{
    assert(!obj.empty());

    WidgetDesc desc;
    for(auto const & kvp: obj)
    {
        if(kvp.key() == WidgetDesc::sid_minimal_size)
        {
            std::vector<int32_t> vec;
            vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

            desc.min_size.x = static_cast<float>(vec[0]);
            desc.min_size.y = static_cast<float>(vec[1]);
        }
        else if(kvp.key() == WidgetDesc::sid_maximal_size)
        {
            std::vector<int32_t> vec;
            vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

            desc.max_size.x = static_cast<float>(vec[0]);
            desc.max_size.y = static_cast<float>(vec[1]);
        }
        else if(kvp.key() == WidgetDesc::sid_type)
        {
            desc.type = WidgetDesc::GetElementTypeFromString(kvp.value().as_string());
        }
        else if(kvp.key() == WidgetDesc::sid_stretch)
        {
            desc.stretch = static_cast<float>(kvp.value().as_int64());
        }
        else if(kvp.key() == WidgetDesc::sid_visible)
        {
            desc.visible = kvp.value().as_bool();
        }
        else if(kvp.key() == WidgetDesc::sid_region_name)
        {
            desc.region_name = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_id_name)
        {
            desc.id_name = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_align_horizontal)
        {
            desc.horizontal = WidgetDesc::GetAlignFromString(kvp.value().as_string());
        }
        else if(kvp.key() == WidgetDesc::sid_align_vertical)
        {
            desc.vertical = WidgetDesc::GetAlignFromString(kvp.value().as_string());
        }
        else if(kvp.key() == WidgetDesc::sid_font)
        {
            desc.font_name = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_font_size)
        {
            desc.size = static_cast<float>(kvp.value().as_int64());
        }
        else if(kvp.key() == WidgetDesc::sid_static_text)
        {
            desc.static_text = kvp.value().as_string();
        }
        else if(kvp.key() == WidgetDesc::sid_text_horizontal)
        {
            desc.text_hor = WidgetDesc::GetAlignFromString(kvp.value().as_string());
        }
    }

    auto widg_ptr = UIWindow::GetWidgetFromDesc(desc, owner);

    if(auto const children_it = obj.find(WidgetDesc::sid_children); children_it != obj.end())
    {
        auto const & arr = children_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & child_entry: arr)
            {
                auto const & widget_obj = child_entry.as_object();
                if(!widget_obj.empty())
                {
                    widg_ptr->addWidget(GetWidgetFromJson(widget_obj, owner));
                }
            }
        }
    }

    return widg_ptr;
}

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
    if(!desc.font_name.empty())
    {
        m_font = ui.m_fonts.getFont(desc.font_name, desc.size);
    }
    else
    {
        // default font
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
