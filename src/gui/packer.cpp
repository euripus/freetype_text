#include "packer.h"
#include "window.h"
#include "basic_types.h"
#include <algorithm>

// -------------------MatrixPacker-------------------
void MatrixPacker::fitWidgets(UIWindow * win) const
{
    if(win == nullptr)
        return;

    auto list = getMatrixFromTree(win->getRootWidget());

    float max_width = 0.f;
    for(auto const & row: list)
        max_width = glm::max(max_width, getRowSumWidth(row));

    adjustWidgetsInRow(win, list, max_width);
}

MatrixPacker::WidgetMatrix MatrixPacker::getMatrixFromTree(Widget * root) const
{
    WidgetMatrix list;

    if(root != nullptr)
        addSubTree(list, root, 0, 0);

    std::reverse(std::begin(list), std::end(list));

    return list;
}

void MatrixPacker::addWidgetPtr(WidgetMatrix & mtx, Widget * ptr, int32_t x, int32_t y) const
{
    if(static_cast<int32_t>(mtx.size()) < y + 1)
        mtx.resize(y + 1);

    if(static_cast<int32_t>(mtx[y].size()) < x + 1)
        mtx[y].resize(x + 1, nullptr);

    mtx[y][x] = ptr;
}

void MatrixPacker::addSubTree(WidgetMatrix & ls, Widget * root, int32_t x, int32_t y) const
{
    enum class Direction
    {
        vertical,
        horizontal,
        not_defined
    };

    if(root == nullptr || root->getType() == ElementType::Unknown)
        return;

    if(root->getType() == ElementType::VerticalLayoutee || root->getType() == ElementType::HorizontalLayoutee)
    {
        Direction dir = Direction::not_defined;

        if(root->getType() == ElementType::VerticalLayoutee)
            dir = Direction::vertical;
        else
            dir = Direction::horizontal;

        for(auto const & ch: root->getChildren())
        {
            addSubTree(ls, ch.get(), x, y);

            switch(ch->getType())
            {
                case ElementType::VerticalLayoutee:
                {
                    if(dir == Direction::vertical)
                        y += ch->getChildren().size();
                    else
                        x += 1;

                    break;
                }
                case ElementType::HorizontalLayoutee:
                {
                    if(dir == Direction::horizontal)
                        x += ch->getChildren().size();
                    else
                        y += 1;

                    break;
                }
                default:
                {
                    if(dir == Direction::vertical)
                        y += 1;
                    else if(dir == Direction::horizontal)
                        x += 1;

                    break;
                }
            }
        }
    }
    else   // if widget - add to matrix
    {
        addWidgetPtr(ls, root, x, y);
    }
}

float MatrixPacker::getRowSumWidth(std::vector<Widget *> const & row) const
{
    float width = m_horizontal_spacing;

    for(auto const * widget: row)
        width += widget->size().x + m_horizontal_spacing;

    return width;
}

float MatrixPacker::getRowMaxHeight(std::vector<Widget *> const & row) const
{
    float height = 0.f;

    for(auto const * widget: row)
        height = glm::max(height, widget->size().y);

    return height;
}

float MatrixPacker::getSumOfFixedWidthInRow(std::vector<Widget *> const & row) const
{
    float result = 0.f;

    for(auto const * widget: row)
        if(widget->getSizePolicy() != SizePolicy::scale)
            result += widget->size().x;

    return result;
}

int32_t MatrixPacker::getNumOfScaledElementsInRow(std::vector<Widget *> const & row) const
{
    int32_t result = 0;

    for(auto const * widget: row)
        if(widget->getSizePolicy() == SizePolicy::scale)
            result++;

    return result;
}

void MatrixPacker::adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const
{
    float current_height = m_vertical_spacing;
    float final_width    = 0.f;

    for(auto & row: ls)
    {
        auto    num_widgets         = row.size();
        float   row_height          = getRowMaxHeight(row);
        float   remaining_width     = new_width - m_horizontal_spacing * (num_widgets + 1);
        int32_t num_scaled_elements = getNumOfScaledElementsInRow(row);
        num_scaled_elements = num_scaled_elements == 0 ? 1 : num_scaled_elements;   // avoid division by zero
        float scaled_element_width = (remaining_width - getSumOfFixedWidthInRow(row)) / num_scaled_elements;
        float current_pos          = m_horizontal_spacing;

        for(auto * widget: row)
        {
            glm::vec2 pos, size;

            if(widget->getSizePolicy() == SizePolicy::scale)
            {   // resizing branch
                pos  = glm::vec2(current_pos, current_height);
                size = glm::vec2(scaled_element_width, row_height);
            }
            else
            {   // fixed size branch, change only position
                pos  = glm::vec2(current_pos, current_height);
                size = widget->getRect().m_extent;
            }

            Rect2D new_rect{pos, size};
            widget->setRect(new_rect);
            current_pos += widget->size().x + m_horizontal_spacing;
        }

        final_width     = glm::max(current_pos, new_width);
        current_height += row_height + m_vertical_spacing;
    }

    if(auto backround = win->getBackgroundWidget(); backround != nullptr)
    {
        auto rect     = backround->getRect();
        rect.m_extent = glm::vec2(final_width, current_height);
        backround->setRect(rect);
    }
    auto rect     = win->getRect();
    rect.m_extent = glm::vec2(final_width, current_height);
    win->setRect(rect);
}

// -------------------TreePacker-------------------
void TreePacker::fitWidgets(UIWindow * win) const {}

void TreePacker::setChildGeometry(Rect2D const & r, Widget * wdg) const {}
