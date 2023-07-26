#ifndef RECT2D_H
#define RECT2D_H

#include <glm/glm.hpp>

struct Rect2D
{
    // Cartesian coordinates
    glm::ivec2 m_pos;
    glm::ivec2 m_extent;

    Rect2D() noexcept = default;

    constexpr Rect2D(glm::ivec2 const & position, glm::ivec2 const & extent) noexcept :
        m_pos(position), m_extent(extent)
    {}

    constexpr Rect2D(const std::int32_t x, const std::int32_t y, const std::int32_t width,
                     const std::int32_t height) noexcept :
        m_pos(x, y), m_extent(width, height)
    {}

    static Rect2D fromLeftBottomRightTop(const std::int32_t left, const std::int32_t bottom,
                                         const std::int32_t right, const std::int32_t top);

    static constexpr Rect2D empty() noexcept { return {}; }

    inline std::int32_t left() const noexcept { return m_pos.x; }

    inline std::int32_t top() const noexcept { return m_pos.y + m_extent.y; }

    inline std::int32_t right() const noexcept { return m_pos.x + m_extent.x; }

    inline std::int32_t bottom() const noexcept { return m_pos.y; }

    inline std::int32_t width() const noexcept { return m_extent.x; }

    inline std::int32_t height() const noexcept { return m_extent.y; }

    bool contains(const std::int32_t x, const std::int32_t y) const noexcept
    {
        return (x >= left() && x < right() && y < top() && y >= bottom());
    }

    bool contains(glm::ivec2 const & value) const noexcept { return contains(value.x, value.y); }

    glm::ivec2 getCenter() const noexcept { return {m_pos.x + m_extent.x / 2, m_pos.y + m_extent.y / 2}; }

    void inflate(const std::int32_t horizontal_value, const std::int32_t vertical_value);

    bool isEmpty() const noexcept
    {
        return (m_pos.x == 0 && m_pos.y == 0 && m_extent.x == 0 && m_extent.y == 0);
    }

    bool intersects(Rect2D const & value) const noexcept
    {
        return left() < value.right() && right() > value.left() && top() > value.bottom()
               && bottom() < value.top();
    }

    static Rect2D intersect(Rect2D const & rect1, Rect2D const & rect2);

    static Rect2D union_rect2D(Rect2D const & rect1, Rect2D const & rect2);

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
