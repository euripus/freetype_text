#include "widget.h"
#include "window.h"
#include "ui.h"

struct WidgetDesc
{
    glm::vec2   size_hint    = {};
    ElementType type         = ElementType::Unknown;
    bool        visible      = true;
    std::string region_name  = {};
    std::string id_name      = {};
    SizePolicy  scale        = SizePolicy::scale;
    Align       horizontal   = Align::left;
    Align       vertical     = Align::top;
    std::string font_name    = {};
    std::string texture_name = {};   // texture name from gui_set
    float       size         = 0.0f;
};

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

Widget::Widget(UIWindow & owner, WidgetDesc const & desc) : Widget(owner)
{
    m_size_hint  = desc.size_hint;
    m_id         = desc.id_name;
    m_region     = desc.region_name;
    m_visible    = desc.visible;
    m_horizontal = desc.horizontal;
    m_vertical   = desc.vertical;
    m_scale      = desc.scale;
    m_type       = desc.type;

    m_font = &m_owner.getOwner().m_fonts.getFont(desc.font_name, desc.size);
    // texture
}

void Widget::update(float time, bool check_cursor) {}

void Widget::draw() {}

void Widget::addWidget(std::unique_ptr<Widget> widget)
{
    widget->m_parent = this;
    m_children.push_back(std::move(widget));
}

void Widget::removeWidget(Widget * widget) {}

bool Widget::isChild(Widget * widget)
{
    return false;
}

std::unique_ptr<Widget> Widget::GetWidgetFromDesc(boost::json::object const & obj, UIWindow & owner)
{
    assert(!obj.empty());

    WidgetDesc desc;
    for(auto const & kvp : obj)
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
    }

    auto widg_ptr = std::make_unique<Widget>(owner, desc);

    auto const children_it = obj.find(sid_children);
    assert(children_it != obj.end());

    auto const & arr = children_it->value().as_array();
    if(!arr.empty())
    {
        for(auto const & child_entry : arr)
        {
            auto const & widget_obj = child_entry.as_object();
            if(!widget_obj.empty())
            {
                widg_ptr->addWidget(GetWidgetFromDesc(widget_obj, owner));
            }
        }
    }

    return widg_ptr;
}
