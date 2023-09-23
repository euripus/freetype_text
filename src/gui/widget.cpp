#include "widget.h"

void Widget::update(float time, bool check_cursor) {}

void Widget::draw() {}

void Widget::addWidget(std::unique_ptr<Widget> widget, Align align) 
{
	m_children.push_back(std::move(widget));
	m_align = align;
	
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
