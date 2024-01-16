#include "packer.h"
#include "window.h"
#include "basic_types.h"
#include <algorithm>

void Packer::fitWidgets(UIWindow * win) const
{
    if(win == nullptr)
        return;

    auto list = getMatrixFromTree(win->m_root.get());

    float max_width = 0.0f;
    for(auto const & row: list)
        max_width = glm::max(max_width, getRowMaxWidth(row));

    adjustWidgetsInRow(win, list, max_width);
}

Packer::WidgetMatrix Packer::getMatrixFromTree(Widget * root) const
{
    WidgetMatrix list;

    if(root != nullptr)
        addSubTree(list, root);

    std::reverse(std::begin(list), std::end(list));

    return list;
}

void Packer::addSubTree(WidgetMatrix & ls, Widget * root, std::uint32_t level) const
{
    if(root == nullptr || root->m_type == ElementType::Unknown)
        return;

    std::vector<Widget *> * row = nullptr;
    if(level < ls.size())
    {
        row = &ls[level];
    }
    else
    {
        ls.push_back({});
        row = &ls.back();
    }

    auto & ch_list = *row;

    if(root->m_type == ElementType::VerticalLayoutee || root->m_type == ElementType::HorizontalLayoutee)
    {
        for(auto const & ch: root->m_children)
        {
            if(auto * cur_ch_ptr = ch.get(); cur_ch_ptr->m_type == ElementType::VerticalLayoutee
                                             || cur_ch_ptr->m_type == ElementType::HorizontalLayoutee)
            {
                if(cur_ch_ptr->m_type == ElementType::HorizontalLayoutee)
                    addSubTree(ls, cur_ch_ptr, level + 1);
                else
                {
                    for(auto const & ch2: cur_ch_ptr->m_children)
                    {
                        addSubTree(ls, ch2.get(), level + 1);
                    }
                }
            }
            else
            {
                ch_list.push_back(cur_ch_ptr);
            }
        }
    }
    else
    {
        ch_list.push_back(root);
    }
}

float Packer::getRowMaxWidth(std::vector<Widget *> const & row) const
{
    float width = m_horizontal_spacing;

    for(auto const * widget: row)
        width += widget->size().x + m_horizontal_spacing;

    return width;
}

float Packer::getRowMaxHeight(std::vector<Widget *> const & row) const
{
    float height = 0.0f;

    for(auto const * widget: row)
        height = glm::max(height, widget->size().y);

    return height;
}

void Packer::adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const
{
    float current_height = m_vertical_spacing;
    float final_width    = 0.0f;

    for(auto & row: ls)
    {
        auto  num_widgets   = row.size();
        float row_height    = getRowMaxHeight(row);
        float element_width = (new_width - m_horizontal_spacing * (num_widgets + 1)) / num_widgets;
        float current_pos   = m_horizontal_spacing;

        for(auto * widget: row)
        {
            switch(widget->m_scale)
            {
                case SizePolicy::scale:   // resizing branch
                {
                    glm::vec2 pos(current_pos, current_height);
                    glm::vec2 size(element_width, row_height);

                    Rect2D new_rect{pos, size};
                    widget->m_rect = new_rect;

                    break;
                }
                case SizePolicy::none:   // fixed size branch, change only position
                case SizePolicy::fixed_width:
                case SizePolicy::fixed_height:
                case SizePolicy::trim:
                {
                    glm::vec2 size(widget->size());

                    if(size.x < element_width)
                    {
                        // vertical align
                        float vertical_delta = row_height - size.y;
                        float widget_y       = current_height;
                        if(vertical_delta > 0)
                        {
                            if(widget->m_vertical == Align::top)
                                widget_y += vertical_delta;
                            else if(widget->m_vertical == Align::center)
                                widget_y += vertical_delta / 2.0f;
                        }

                        // horizontal align
                        float horizontal_delta = element_width - size.x;
                        float widget_x         = current_pos;
                        if(horizontal_delta > 0)
                        {
                            if(widget->m_horizontal == Align::right)
                                widget_x += horizontal_delta;
                            else if(widget->m_horizontal == Align::center)
                                widget_x += horizontal_delta / 2.0f;
                        }

                        glm::vec2 pos(widget_x, widget_y);

                        Rect2D new_rect{pos, size};
                        widget->m_rect = new_rect;
                    }
                    else
                    {
                        // vertical align
                        float vertical_delta = row_height - size.y;
                        float widget_y       = current_height;
                        if(vertical_delta > 0)
                        {
                            if(widget->m_vertical == Align::top)
                                widget_y += vertical_delta;
                            else if(widget->m_vertical == Align::center)
                                widget_y += vertical_delta / 2.0f;
                        }

                        glm::vec2 pos(current_pos, widget_y);

                        Rect2D new_rect{pos, size};
                        widget->m_rect = new_rect;
                    }

                    break;
                }
            }

            current_pos += widget->size().x + m_horizontal_spacing;
        }

        final_width     = glm::max(current_pos, new_width);
        current_height += row_height + m_vertical_spacing;
    }

    win->m_rect.m_extent = glm::vec2(final_width, current_height);
}
