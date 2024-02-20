struct unit
{
    int         initiative;   // in percent
    float       internal;     // initiative/100.f
    float       head_start;   // in seconds [0 ... 1/internal]
    std::string name;
};

std::array<float, 10> getTurnTimes(unit const & u, float const actual_time = 0.f)
{
    std::array<float, 10> result;
    float const           sec_for_turn = 1.f / u.internal;
    float const           curr_turn    = std::floor(actual_time / sec_for_turn);

    for(uint32_t i = curr_turn; i < curr_turn + result.size(); ++i)
    {
        result[i] = std::max(i / u.internal - head_start, 0.f);
    }

    return result;
}

//========================================================================//
enum class ElementType
{
    collumn,
    row,
    widget
};

enum class Direction
{
    vertical,
    horizontal,
    not_defined
};

struct element
{
    ElementType              type;
    std::vector<element_ptr> elements;
    Widget *                 ptr = nullptr;

    glm::ivec2 size() const
    {
        if(ptr != nullptr)
            return {1, 1};
        else
        {
            if(type == ElementType::collumn)
                return {1, widget_set.size()};
            else
                return {widget_set.size(), 1};
        }
    }
};

using WidgetList   = std::vector<element>;
using WidgetMatrix = std::vector<std::vector<Widget *>>;

WidgetList list = getWidgetListFromTree(...);

void addElement(WidgetMatrix & mtx, element & el, int32_t x, int32_t y);

WidgetMatrix getWidgetMatrix(element const & root)
{
    WidgetMatrix result;

    if(root.type == ElementType::widget)
    {
        addElement(result, root, 0, 0);
        return result;
    }
    else
    {
        Direction dir = Direction::not_defined;
        int32_t   x   = 0;
        int32_t   y   = 0;

        if(root.type == ElementType::collumn)
            dir = Direction::vertical;
        else
            dir = Direction::horizontal;

        for(auto const & el: root.elements)
        {
            addElement(result, el, x, y);

            switch(el.type)
            {
                case ElementType::widget:
                {
                    if(dir == Direction::vertical)
                        y += 1;
                    else if(dir == Direction::horizontal)
                        x += 1;

                    break;
                }
                case ElementType::collumn:
                {
                    y += el.size().y;
                    break;
                }
                case ElementType::row:
                {
                    x += el.size().x;
                    break;
                }
            }
        }
    }

    return result;
}
