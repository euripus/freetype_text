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

void addWidgetPtr(WidgetMatrix & mtx, Widget const * ptr, int32_t x, int32_t y);

void addElement(WidgetMatrix & mtx, element & el, int32_t x, int32_t y)
{
	if(el.type == ElementType::widget)
    {
        addWidgetPtr(result, el.ptr, x, y);
    }
    else
    {
        Direction dir = Direction::not_defined;

        if(el.type == ElementType::collumn)
            dir = Direction::vertical;
        else
            dir = Direction::horizontal;

        for(auto const & elm: el.elements)
        {
            addElement(result, elm, x, y);

            switch(elm.type)
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
                    y += elm.size().y;
                    break;
                }
                case ElementType::row:
                {
                    x += elm.size().x;
                    break;
                }
            }
        }
    }
}

WidgetMatrix getWidgetMatrix(element const & root)
{
    WidgetMatrix result;

	addElement(result, root, 0, 0);

    return result;
}
