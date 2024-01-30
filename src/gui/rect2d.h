#ifndef RECT2D_H
#define RECT2D_H

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

struct Rect2D
{
    // Cartesian coordinates
    glm::vec2 m_pos    = {0.f, 0.f};
    glm::vec2 m_extent = {0.f, 0.f};

    Rect2D() noexcept = default;

    constexpr Rect2D(glm::vec2 const & position, glm::vec2 const & extent) noexcept
        : m_pos(position),
          m_extent(extent)
    {}

    constexpr Rect2D(float const x, float const y, float const width, float const height) noexcept
        : m_pos(x, y),
          m_extent(width, height)
    {}

    static Rect2D FromLeftBottomRightTop(float const left, float const bottom, float const right,
                                         float const top);

    static constexpr Rect2D Empty() noexcept { return {}; }

    inline float left() const noexcept { return m_pos.x; }

    inline float top() const noexcept { return m_pos.y + m_extent.y; }

    inline float right() const noexcept { return m_pos.x + m_extent.x; }

    inline float bottom() const noexcept { return m_pos.y; }

    inline float width() const noexcept { return m_extent.x; }

    inline float height() const noexcept { return m_extent.y; }

    bool contains(float const x, float const y) const noexcept
    {
        return (x >= left() && x < right() && y < top() && y >= bottom());
    }

    bool contains(glm::vec2 const & value) const noexcept { return contains(value.x, value.y); }

    glm::vec2 getCenter() const noexcept { return {m_pos.x + m_extent.x / 2, m_pos.y + m_extent.y / 2}; }

    void inflate(float const horizontal_value, float const vertical_value);

    bool isEmpty() const noexcept
    {
        auto const cmp_ext =
           glm::epsilonEqual(m_extent, glm::vec2(0.f, 0.f), std::numeric_limits<float>::epsilon());
        auto const cmp_pos =
           glm::epsilonEqual(m_pos, glm::vec2(0.f, 0.f), std::numeric_limits<float>::epsilon());
        return (cmp_ext.x && cmp_ext.y && cmp_pos.x && cmp_pos.y == 0);
    }

    bool intersects(Rect2D const & value) const noexcept
    {
        return left() < value.right() && right() > value.left() && top() > value.bottom()
               && bottom() < value.top();
    }

    static Rect2D Intersect(Rect2D const & rect1, Rect2D const & rect2);

    static Rect2D Union_rect2D(Rect2D const & rect1, Rect2D const & rect2);

    bool operator==(Rect2D const & rhs) const noexcept
    {
        return (m_pos == rhs.m_pos && m_extent == rhs.m_extent);
    }

    bool operator!=(Rect2D const & rhs) const noexcept
    {
        return (m_pos != rhs.m_pos || m_extent != rhs.m_extent);
    }
};

#endif   // RECT2D_H
