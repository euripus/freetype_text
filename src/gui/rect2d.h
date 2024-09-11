#ifndef RECT2D_H
#define RECT2D_H

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

struct Rect2D
{
    // Cartesian coordinates
    glm::vec2 m_pos  = {0.f, 0.f};
    glm::vec2 m_size = {0.f, 0.f};

    Rect2D() noexcept = default;

    constexpr Rect2D(glm::vec2 const & position, glm::vec2 const & extent) noexcept
        : m_pos(position),
          m_size(extent)
    {}

    constexpr Rect2D(float const x, float const y, float const width, float const height) noexcept
        : m_pos(x, y),
          m_size(width, height)
    {}

    static Rect2D FromLeftBottomRightTop(float const left, float const bottom, float const right,
                                         float const top);

    static constexpr Rect2D Empty() noexcept { return {}; }

    inline float left() const noexcept { return m_pos.x; }

    inline float top() const noexcept { return m_pos.y + m_size.y; }

    inline float right() const noexcept { return m_pos.x + m_size.x; }

    inline float bottom() const noexcept { return m_pos.y; }

    inline float width() const noexcept { return m_size.x; }

    inline float height() const noexcept { return m_size.y; }

    bool contains(float const x, float const y) const noexcept
    {
        return (x >= left() && x < right() && y < top() && y >= bottom());
    }

    bool contains(glm::vec2 const & value) const noexcept { return contains(value.x, value.y); }

    glm::vec2 getCenter() const noexcept { return {m_pos.x + m_size.x / 2, m_pos.y + m_size.y / 2}; }

    void inflate(float const horizontal_value, float const vertical_value);

    bool isEmpty() const noexcept
    {
        auto const cmp_size =
            glm::all(glm::epsilonEqual(m_size, glm::vec2(0.f, 0.f), std::numeric_limits<float>::epsilon()));
        auto const cmp_pos =
            glm::all(glm::epsilonEqual(m_pos, glm::vec2(0.f, 0.f), std::numeric_limits<float>::epsilon()));
        return (cmp_size && cmp_pos);
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
        auto const cmp_size =
            glm::all(glm::epsilonEqual(m_size, rhs.m_size, std::numeric_limits<float>::epsilon()));
        auto const cmp_pos =
            glm::all(glm::epsilonEqual(m_pos, rhs.m_pos, std::numeric_limits<float>::epsilon()));
        return (cmp_size && cmp_pos);
    }

    bool operator!=(Rect2D const & rhs) const noexcept { return !(*this == rhs); }
};

#endif   // RECT2D_H
