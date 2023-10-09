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
    for(auto const & row : list)
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

void Packer::addSubTree(WidgetMatrix & ls, Widget * root) const
{
    if(root == nullptr || root->m_type == ElementType::Unknown)
        return;

    ls.push_back({});
    auto & ch_list = ls.back();

    if(root->m_type == ElementType::VerticalLayoutee || root->m_type == ElementType::HorizontalLayoutee)
    {
        for(auto const & ch : root->m_children)
        {
            if(auto * cur_ch_ptr = ch.get(); cur_ch_ptr->m_type == ElementType::VerticalLayoutee
                                             || cur_ch_ptr->m_type == ElementType::HorizontalLayoutee)
            {
                if(cur_ch_ptr->m_type == ElementType::HorizontalLayoutee)
                    addSubTree(ls, cur_ch_ptr);
                else
                {
                    for(auto const & ch2 : cur_ch_ptr->m_children)
                    {
                        addSubTree(ls, ch2.get());
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

    for(auto const * widget : row)
        width += widget->size().x + m_horizontal_spacing;

    return width;
}

float Packer::getRowMaxHeight(std::vector<Widget *> const & row) const
{
    float height = 0.0f;

    for(auto const * widget : row)
        height = glm::max(height, widget->size().y);

    return height;
}

void Packer::adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const
{
    // 1. Rearrange widgets in rows
    // 2. Set new_size and new_pos for window widgets

    float current_height = m_vertical_spacing;

    for(auto & row : ls)
    {
        auto  num_widgets   = row.size();
        float row_height    = getRowMaxHeight(row);
        float element_width = new_width / num_widgets;
        float current_pos   = m_horizontal_spacing;

        for(auto * widget : row)
        {
            glm::vec2 pos(current_pos, current_height);

            if(widget->m_scale == SizePolicy::scale) {}
            else if(widget->m_scale == SizePolicy::none) {}
        }

        current_height = row_height + m_vertical_spacing;
    }

    win->m_size = glm::vec2(new_width, current_height);
}
