#include "widget.h"
#include "window.h"
#include "ui.h"
#include <algorithm>
#include <glm/gtc/epsilon.hpp>

#include "text_box.h"
#include "button.h"
#include "../vertex_buffer.h"

ElementType Widget::GetElementTypeFromString(std::string_view name)
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
    else if(name == "RadioButton")
        type = ElementType::RadioButton;
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

SizePolicy Widget::GetSizePolicyFromString(std::string_view name)
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

Align Widget::GetAlignFromString(std::string_view name)
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

Widget::Widget(WidgetDesc const & desc, UIWindow & owner)
    : m_owner(owner)
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

    if(!m_region_name.empty())
        m_region_ptr = ui.getImageRegion(m_region_name);
}

void Widget::update(float time, bool check_cursor)
{
    if(check_cursor)
    {}
}

void Widget::draw(VertexBuffer & background, VertexBuffer & text) const
{
    if(m_region_ptr != nullptr && visible())
    {
        glm::vec2 pos = m_pos;
        m_region_ptr->addBlock(background, pos, m_rect.m_extent);
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

/*glm::vec2 Widget::size() const
{
    // auto const cmp =
    //     glm::epsilonEqual(m_rect.m_extent, glm::vec2(0.f, 0.f), std::numeric_limits<float>::epsilon());
    // if(cmp.x || cmp.y)
    //     return sizeHint();

    return m_rect.m_extent;
}*/

std::unique_ptr<Widget> Widget::GetWidgetFromDesc(WidgetDesc const & desc, UIWindow & owner)
{
    std::unique_ptr<Widget> widg_ptr;

    switch(desc.type)
    {
        case ElementType::TextBox:
        {
            widg_ptr = std::make_unique<TextBox>(desc, owner);

            break;
        }
        // case ElementType::ImageBox:
        // {
        //     // widg_ptr = std::make_unique<ImageBox>(std::string(), owner);
        //     break;
        // }
        case ElementType::Button:
        {
            widg_ptr = std::make_unique<Button>(desc, owner);

            break;
        }
        case ElementType::VerticalLayoutee:
        case ElementType::HorizontalLayoutee:
        case ElementType::Unknown:
        case ElementType::ImageBox:
        {
            widg_ptr = std::make_unique<Widget>(desc, owner);
            break;
        }
    }

    return widg_ptr;
}

std::unique_ptr<Widget> Widget::GetWidgetFromDesc(boost::json::object const & obj, UIWindow & owner)
{
    assert(!obj.empty());

    WidgetDesc desc;
    for(auto const & kvp: obj)
    {
        if(kvp.key() == sid_size)
        {
            std::vector<int32_t> vec;
            vec = boost::json::value_to<std::vector<int32_t>>(kvp.value());

            desc.size_hint.x = static_cast<float>(vec[0]);
            desc.size_hint.y = static_cast<float>(vec[1]);
        }
        else if(kvp.key() == sid_type)
        {
            desc.type = GetElementTypeFromString(kvp.value().as_string());
        }
        else if(kvp.key() == sid_visible)
        {
            desc.visible = kvp.value().as_bool();
        }
        else if(kvp.key() == sid_texture)
        {
            desc.texture_name = kvp.value().as_string();
        }
        else if(kvp.key() == sid_region_name)
        {
            desc.region_name = kvp.value().as_string();
        }
        else if(kvp.key() == sid_id_name)
        {
            desc.id_name = kvp.value().as_string();
        }
        else if(kvp.key() == sid_size_policy)
        {
            desc.scale = GetSizePolicyFromString(kvp.value().as_string());
        }
        else if(kvp.key() == sid_align_horizontal)
        {
            desc.horizontal = GetAlignFromString(kvp.value().as_string());
        }
        else if(kvp.key() == sid_align_vertical)
        {
            desc.vertical = GetAlignFromString(kvp.value().as_string());
        }
        else if(kvp.key() == sid_font)
        {
            desc.font_name = kvp.value().as_string();
        }
        else if(kvp.key() == sid_font_size)
        {
            desc.size = static_cast<float>(kvp.value().as_int64());
        }
        else if(kvp.key() == sid_static_text)
        {
            desc.static_text = kvp.value().as_string();
        }
        else if(kvp.key() == sid_text_horizontal)
        {
            desc.text_hor = GetAlignFromString(kvp.value().as_string());
        }
    }

    auto widg_ptr = GetWidgetFromDesc(desc, owner);

    if(auto const children_it = obj.find(sid_children); children_it != obj.end())
    {
        auto const & arr = children_it->value().as_array();
        if(!arr.empty())
        {
            for(auto const & child_entry: arr)
            {
                auto const & widget_obj = child_entry.as_object();
                if(!widget_obj.empty())
                {
                    widg_ptr->addWidget(GetWidgetFromDesc(widget_obj, owner));
                }
            }
        }
    }

    return widg_ptr;
}
