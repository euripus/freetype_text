#include "packer.h"
#include "window.h"
#include "basic_types.h"
#include <algorithm>

template<typename T>
T const & GetRef(std::unique_ptr<T> const & ptr)
{
    if(ptr)
    {
        return *ptr.get();
    }

    throw std::runtime_error("Error dereferencing null unique_ptr!");
}

template<typename T>
T & GetRef(std::unique_ptr<T> & ptr)
{
    if(ptr)
    {
        return *ptr.get();
    }

    throw std::runtime_error("Error dereferencing null unique_ptr!");
}

glm::vec2 Packer::getWidgetSize(Widget const & w, std::function<glm::vec2(Widget const &)> size_func) const
{
    glm::vec2 result{0.0f};

    switch(w.getType())
    {
        case ElementType::VerticalLayoutee:
            {
                for(auto const & ch : w.getChildren())
                {
                    auto const child_size = getWidgetSize(GetRef(ch), size_func);

                    result.x = std::max(result.x, child_size.x);
                    result.y += child_size.y;
                }

                if(w.getNumChildren() > 1)
                {
                    result.y += m_horizontal_spacing * (w.getNumChildren() - 1);
                }

                break;
            }
        case ElementType::HorizontalLayoutee:
            {
                for(auto const & ch : w.getChildren())
                {
                    auto const child_size = getWidgetSize(GetRef(ch), size_func);

                    result.x += child_size.x;
                    result.y = std::max(result.y, child_size.y);
                }

                if(w.getNumChildren() > 1)
                {
                    result.x += m_vertical_spacing * (w.getNumChildren() - 1);
                }

                break;
            }
        default:
            {
                result = size_func(w);

                break;
            }
    }

    return result;
}

// -------------------MatrixPacker-------------------
void MatrixPacker::fitWidgets(UIWindow * win) const
{
    if(win == nullptr)
        return;

    auto list = getMatrixFromTree(win->getRootWidget());

    float max_width = 0.f;
    for(auto const & row : list)
        max_width = glm::max(max_width, getRowSumWidth(row));

    adjustWidgetsInRow(win, list, max_width);

    auto w_rect = win->getRect();
    if(auto backround = win->getBackgroundWidget(); backround != nullptr)
    {
        auto rect     = backround->getRect();
        rect.m_extent = w_rect.m_extent;
        backround->setRect(rect);
    }
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

        for(auto const & ch : root->getChildren())
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

    for(auto const * widget : row)
        width += widget->size().x + m_horizontal_spacing;

    return width;
}

float MatrixPacker::getRowMaxHeight(std::vector<Widget *> const & row) const
{
    float height = 0.f;

    for(auto const * widget : row)
        height = glm::max(height, widget->size().y);

    return height;
}

float MatrixPacker::getSumOfFixedWidthInRow(std::vector<Widget *> const & row) const
{
    float result = 0.f;

    for(auto const * widget : row)
        if(widget->getSizePolicy() == SizePolicy::fixed_width
           || widget->getSizePolicy() == SizePolicy::fixed_size)
            result += widget->size().x;

    return result;
}

int32_t MatrixPacker::getNumOfScaledElementsInRow(std::vector<Widget *> const & row) const
{
    int32_t result = 0;

    for(auto const * widget : row)
        if(widget->getSizePolicy() == SizePolicy::scalable)
            result++;

    return result;
}

void MatrixPacker::adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const
{
    float current_height = m_vertical_spacing;
    float final_width    = 0.f;

    for(auto & row : ls)
    {
        auto    num_widgets         = row.size();
        float   row_height          = getRowMaxHeight(row);
        float   remaining_width     = new_width - m_horizontal_spacing * (num_widgets + 1);
        int32_t num_scaled_elements = getNumOfScaledElementsInRow(row);
        num_scaled_elements = num_scaled_elements == 0 ? 1 : num_scaled_elements;   // avoid division by zero
        float scaled_element_width = (remaining_width - getSumOfFixedWidthInRow(row)) / num_scaled_elements;
        float current_pos          = m_horizontal_spacing;

        for(auto * widget : row)
        {
            glm::vec2 pos, size;

            if(widget->getSizePolicy() == SizePolicy::scalable)
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

        final_width = glm::max(current_pos, new_width);
        current_height += row_height + m_vertical_spacing;
    }

    auto rect     = win->getRect();
    rect.m_extent = glm::vec2(final_width, current_height);
    win->setRect(rect);
}

// -------------------TreePacker-------------------
void TreePacker::fitWidgets(UIWindow * win) const
{
    if(win->getRootWidget() == nullptr)
        return;

    Widget &        root            = *win->getRootWidget();
    glm::vec2 const cur_window_size = getWidgetSize(root, [](Widget const & w) { return w.size(); });
    glm::vec2       top_left        = {0, cur_window_size.y};

    if(root.getType() == ElementType::VerticalLayoutee || root.getType() == ElementType::HorizontalLayoutee)
    {
        if(root.getType() == ElementType::VerticalLayoutee)
            arrangeWidgetsInColumn(root, top_left, cur_window_size);
        else
            arrangeWidgetsInRow(root, top_left, cur_window_size);
    }
    else
    {
        placeWidgetInCell(root, top_left, cur_window_size);
    }

    auto w_rect     = root.getRect();
    w_rect.m_extent = cur_window_size;
    win->setRect(w_rect);

    if(auto backround = win->getBackgroundWidget(); backround != nullptr)
    {
        auto rect     = backround->getRect();
        rect.m_extent = w_rect.m_extent;
        backround->setRect(rect);
    }
}

glm::vec2 TreePacker::arrangeWidgetsInRow(Widget & row_node, glm::vec2 cur_tlpos,
                                          glm::vec2 const & win_size) const
{
    if(row_node.getType() != ElementType::VerticalLayoutee
       && row_node.getType() != ElementType::HorizontalLayoutee)
        return {};

    glm::vec2   ret_size              = {0.f, 0.f};
    auto const  row_prop              = getGroupNodeProperties(row_node);
    float const available_width       = win_size.x - row_prop.fixed_elements_size.x;
    float const num_scalable_elements = (row_prop.num_children - row_prop.num_fixed_size_elements) > 0
                                            ? row_prop.num_children - row_prop.num_fixed_size_elements
                                            : 1.f;
    float const scaled_widget_width =
        (available_width - m_horizontal_spacing * (num_scalable_elements - 1.f)) / num_scalable_elements;

    for(auto & ch : row_node.getChildren())
    {
        Widget &   w               = GetRef(ch);
        auto const cur_widget_size = getWidgetSize(w, [](Widget const & w) { return w.size(); });
        glm::vec2  size            = {0.f, 0.f};
        glm::vec2  desired_size    = {0.f, 0.f};

        switch(w.getType())
        {
            case ElementType::VerticalLayoutee:
                {
                    auto const scalable = isGroupNodeScalable(w);

                    if(scalable)
                        desired_size = glm::vec2{scaled_widget_width, win_size.y};
                    else
                        desired_size = glm::vec2{cur_widget_size.x, win_size.y};

                    size = arrangeWidgetsInColumn(w, cur_tlpos, desired_size);

                    break;
                }
            case ElementType::HorizontalLayoutee:
                {
                    auto const scalable = isGroupNodeScalable(w);

                    if(scalable)
                        desired_size = glm::vec2{scaled_widget_width, win_size.y};
                    else
                        desired_size = glm::vec2{cur_widget_size.x, win_size.y};

                    size = arrangeWidgetsInRow(w, cur_tlpos, desired_size);

                    break;
                }
            default:
                {
                    glm::vec2 desired_size = glm::vec2{scaled_widget_width, win_size.y};

                    placeWidgetInCell(w, cur_tlpos, desired_size);
                    size = w.size();

                    break;
                }
        }

        cur_tlpos.x += (size.x + m_horizontal_spacing);
        ret_size.x += (size.x + m_horizontal_spacing);
        ret_size.y = std::max(size.y, ret_size.y);
    }

    row_node.setSize(ret_size);

    return ret_size;
}

glm::vec2 TreePacker::arrangeWidgetsInColumn(Widget & column_node, glm::vec2 cur_tlpos,
                                             glm::vec2 const & win_size) const
{
    if(column_node.getType() != ElementType::VerticalLayoutee
       && column_node.getType() != ElementType::HorizontalLayoutee)
        return {};

    glm::vec2   ret_size              = {0.f, 0.f};
    auto const  column_prop           = getGroupNodeProperties(column_node);
    float const available_height      = win_size.y - column_prop.fixed_elements_size.y;
    float const num_scalable_elements = (column_prop.num_children - column_prop.num_fixed_size_elements) > 0
                                            ? column_prop.num_children - column_prop.num_fixed_size_elements
                                            : 1.f;
    float const scaled_widget_height =
        (available_height - m_vertical_spacing * (num_scalable_elements - 1.f)) / num_scalable_elements;

    for(auto & ch : column_node.getChildren())
    {
        Widget &   w               = GetRef(ch);
        auto const cur_widget_size = getWidgetSize(w, [](Widget const & w) { return w.size(); });
        glm::vec2  size            = {0.f, 0.f};
        glm::vec2  desired_size    = {0.f, 0.f};

        switch(w.getType())
        {
            case ElementType::VerticalLayoutee:
                {
                    auto const scalable = isGroupNodeScalable(w);

                    if(scalable)
                        desired_size = glm::vec2{win_size.x, scaled_widget_height};
                    else
                        desired_size = glm::vec2{win_size.x, cur_widget_size.y};

                    size = arrangeWidgetsInColumn(w, cur_tlpos, desired_size);

                    break;
                }
            case ElementType::HorizontalLayoutee:
                {
                    auto const scalable = isGroupNodeScalable(w);

                    if(scalable)
                        desired_size = glm::vec2{win_size.x, scaled_widget_height};
                    else
                        desired_size = glm::vec2{win_size.x, cur_widget_size.y};

                    size = arrangeWidgetsInRow(w, cur_tlpos, desired_size);

                    break;
                }
            default:
                {
                    desired_size = glm::vec2{win_size.x, scaled_widget_height};

                    placeWidgetInCell(w, cur_tlpos, desired_size);
                    size = w.size();

                    break;
                }
        }

        cur_tlpos.y -= (size.y + m_vertical_spacing);
        ret_size.x = std::max(size.x, ret_size.x);
        ret_size.y += (size.y + m_vertical_spacing);
    }

    column_node.setSize(ret_size);

    return ret_size;
}

void TreePacker::placeWidgetInCell(Widget & w, glm::vec2 top_left_pos, glm::vec2 desired_size) const
{
    float const min_width  = w.sizeHint().x;
    float const min_height = w.sizeHint().y;

    float width  = desired_size.x < min_width ? min_width : desired_size.x;
    float height = desired_size.y < min_height ? min_height : desired_size.y;

    float const vert_free_space  = height - min_height;
    float const horiz_free_space = width - min_width;

    switch(w.getSizePolicy())
    {
        case SizePolicy::fixed_width:
            {
                width = min_width;
                break;
            }
        case SizePolicy::fixed_height:
            {
                height = min_height;
                break;
            }
        case SizePolicy::fixed_size:
            {
                width  = min_width;
                height = min_height;
                break;
            }
    }

    float bottom_x_pos = top_left_pos.x;
    float bottom_y_pos = top_left_pos.y - height;

    switch(w.getVerticalAlign())
    {
        case Align::top:
            {
                bottom_y_pos += vert_free_space;
                break;
            }
        case Align::center:
            {
                bottom_y_pos += vert_free_space / 2.f;
                break;
            }
    }

    switch(w.getHorizontalAlign())
    {
        case Align::center:
            {
                bottom_x_pos += horiz_free_space / 2.f;
                break;
            }
        case Align::right:
            {
                bottom_x_pos += horiz_free_space;
                break;
            }
    }

    w.setRect(Rect2D{bottom_x_pos, bottom_y_pos, width, height});
}

bool TreePacker::isGroupNodeScalable(Widget const & node) const
{
    assert(
        node.getType() == ElementType::HorizontalLayoutee || node.getType() == ElementType::VerticalLayoutee);

    bool result = false;

    if(node.getType() == ElementType::HorizontalLayoutee || node.getType() == ElementType::VerticalLayoutee)
    {
        for(auto const & ch : node.getChildren())
        {
            Widget const & w           = GetRef(ch);
            bool           is_scalable = false;

            switch(w.getType())
            {
                case ElementType::VerticalLayoutee:
                case ElementType::HorizontalLayoutee:
                    {
                        is_scalable = isGroupNodeScalable(w);

                        break;
                    }
                default:
                    {
                        auto const policy = w.getSizePolicy();

                        if(node.getType() == ElementType::HorizontalLayoutee
                           && (policy == SizePolicy::scalable || policy == SizePolicy::fixed_height))
                            is_scalable = true;

                        if(node.getType() == ElementType::VerticalLayoutee
                           && (policy == SizePolicy::scalable || policy == SizePolicy::fixed_width))
                            is_scalable = true;

                        break;
                    }
            }

            result = result || is_scalable;
        }
    }

    return result;
}

TreePacker::GrupNodeProp TreePacker::getGroupNodeProperties(Widget const & node) const
{
    assert(
        node.getType() == ElementType::HorizontalLayoutee || node.getType() == ElementType::VerticalLayoutee);

    GrupNodeProp result;

    if(node.getType() == ElementType::HorizontalLayoutee || node.getType() == ElementType::VerticalLayoutee)
    {
        result.is_horizontal = node.getType() == ElementType::HorizontalLayoutee;
        result.is_scalable   = isGroupNodeScalable(node);
        result.num_children  = node.getChildren().size();
        result.size_hint     = getWidgetSize(node, [](Widget const & w) { return w.sizeHint(); });
        result.size          = getWidgetSize(node, [](Widget const & w) { return w.size(); });

        // fixed size widgets calculation
        if(result.is_scalable == false)
        {
            result.num_fixed_size_elements = result.num_children;
            result.fixed_elements_size     = result.size;
        }
        else
        {
            for(auto const & ch : node.getChildren())
            {
                Widget const & w         = GetRef(ch);
                auto const     policy    = w.getSizePolicy();
                auto const     node_size = getWidgetSize(w, [](Widget const & w) { return w.size(); });

                if(w.getType() == ElementType::HorizontalLayoutee
                   || w.getType() == ElementType::VerticalLayoutee)
                {   // group node
                    if(!isGroupNodeScalable(w))
                    {
                        result.num_fixed_size_elements += 1;

                        if(w.getType() == ElementType::VerticalLayoutee)
                        {
                            result.fixed_elements_size.x =
                                std::max(result.fixed_elements_size.x, node_size.x);
                            result.fixed_elements_size.y += (node_size.y + m_horizontal_spacing);
                        }
                        else
                        {
                            result.fixed_elements_size.x += (node_size.x + m_vertical_spacing);
                            result.fixed_elements_size.y =
                                std::max(result.fixed_elements_size.y, node_size.y);
                        }
                    }
                }
                else
                {   // single node
                    if(result.is_horizontal)
                    {
                        if(policy == SizePolicy::fixed_width || policy == SizePolicy::fixed_size)
                        {
                            result.num_fixed_size_elements += 1;

                            result.fixed_elements_size.x += (node_size.x + m_vertical_spacing);
                            result.fixed_elements_size.y =
                                std::max(result.fixed_elements_size.y, node_size.y);
                        }
                    }
                    else
                    {
                        if(policy == SizePolicy::fixed_height || policy == SizePolicy::fixed_size)
                        {
                            result.num_fixed_size_elements += 1;

                            result.fixed_elements_size.x =
                                std::max(result.fixed_elements_size.x, node_size.x);
                            result.fixed_elements_size.y += (node_size.y + m_horizontal_spacing);
                        }
                    }
                }
            }
        }
    }

    return result;
}
