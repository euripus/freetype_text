#include "rect2d.h"
#include <stdexcept>

Rect2D Rect2D::FromLeftBottomRightTop(float const left, float const bottom, float const right,
                                      float const top)
{
    if(left > right || top < bottom)
    {
        throw std::invalid_argument("arguments out of range");
    }

    return {glm::vec2(left, bottom), glm::vec2((right - left), (top - bottom))};
}

void Rect2D::inflate(float const horizontal_value, float const vertical_value)
{
    if(horizontal_value <= 0.0f || vertical_value <= 0.0f)
    {
        throw std::invalid_argument("arguments out of range");
    }

    m_pos.x    -= horizontal_value;
    m_pos.y    -= vertical_value;
    m_extent.x += horizontal_value * 2;
    m_extent.y += vertical_value * 2;
}

Rect2D Rect2D::Intersect(Rect2D const & rect1, Rect2D const & rect2)
{
    if(rect1.intersects(rect2))
    {
        float const right  = std::min(rect1.right(), rect2.right());
        float const top    = std::min(rect1.top(), rect2.top());
        float const left   = std::max(rect1.left(), rect2.left());
        float const bottom = std::max(rect1.bottom(), rect2.bottom());
        return FromLeftBottomRightTop(left, bottom, right, top);
    }
    return {};
}

Rect2D Rect2D::Union_rect2D(Rect2D const & rect1, Rect2D const & rect2)
{
    float const left   = std::min(rect1.m_pos.x, rect2.m_pos.x);
    float const bottom = std::min(rect1.m_pos.y, rect2.m_pos.y);
    float const right  = std::max(rect1.right(), rect2.right());
    float const top    = std::max(rect1.top(), rect2.top());
    return FromLeftBottomRightTop(left, bottom, right, top);
}
