#include "widget.h"

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

SizePolicy Widget::GetSizePolicyFromString(std::string_view name) {}

Align Widget::GetAlignFromString(std::string_view name) {}

void Widget::update(float time, bool check_cursor) {}

void Widget::draw() {}

void Widget::addWidget(std::unique_ptr<Widget> widget)
{
    m_children.push_back(std::move(widget));

    if(parent())
    {
        // recalculate size
    }
}

void Widget::removeWidget(Widget * widget) {}

bool Widget::isChild(Widget * widget)
{
    return false;
}
