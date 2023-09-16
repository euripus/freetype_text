#include "widget.h"

void Widget::update(float time, bool check_cursor) {}

void Widget::draw() {}

void Widget::addWidget(std::unique_ptr<Widget> widget, Align align) {}

void Widget::removeWidget(Widget * widget) {}

bool Widget::isChild(Widget * widget)
{
    return false;
}
