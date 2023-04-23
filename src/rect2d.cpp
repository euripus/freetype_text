#include "rect2d.h"
#include <stdexcept>

Rect2D Rect2D::fromLeftBottomRightTop(const int32_t left, const int32_t bottom, const int32_t right,
                                      const int32_t top)
{
    if(left > right || top < bottom)
    {
        throw std::invalid_argument("arguments out of range");
    }

    return {glm::ivec2(left, bottom), glm::ivec2((right - left), (top - bottom))};
}

void Rect2D::inflate(const int32_t horizontal_value, const int32_t vertical_value)
{
    if(horizontal_value <= 0 || vertical_value <= 0)
    {
        throw std::invalid_argument("");
    }

    m_pos.x -= horizontal_value;
    m_pos.y -= vertical_value;
    m_extent.x += horizontal_value * 2;
    m_extent.y += vertical_value * 2;
}

Rect2D Rect2D::intersect(Rect2D const & rect1, Rect2D const & rect2)
{
    if(rect1.intersects(rect2))
    {
        const int32_t right  = std::min(rect1.right(), rect2.right());
        const int32_t top    = std::min(rect1.top(), rect2.top());
        const int32_t left   = std::max(rect1.left(), rect2.left());
        const int32_t bottom = std::max(rect1.bottom(), rect2.bottom());
        return fromLeftBottomRightTop(left, bottom, right, top);
    }
    return {};
}

Rect2D Rect2D::union_rect2D(Rect2D const & rect1, Rect2D const & rect2)
{
    const int32_t left   = std::min(rect1.m_pos.x, rect2.m_pos.x);
    const int32_t bottom = std::min(rect1.m_pos.y, rect2.m_pos.y);
    const int32_t right  = std::max(rect1.right(), rect2.right());
    const int32_t top    = std::max(rect1.top(), rect2.top());
    return fromLeftBottomRightTop(left, bottom, right, top);
}
