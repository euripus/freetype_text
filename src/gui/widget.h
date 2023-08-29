#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Widget
{
public:
    Widget() = default;
    virtual ~Widget();

    virtual void update(float time);
    virtual void draw();

    Widget * parent() const { return m_parent; }

    void show();
    void hide();
    bool visible() const;

    glm::vec2 size() const;

    glm::vec2 pos() const;
    void      move(glm::vec2 const & point);
    void      resize(glm::vec2 const & new_size);

protected:
    glm::vec2 m_size;
    glm::vec2 m_size_min;
    glm::vec2 m_size_desired;
    glm::vec2 m_pos;

    bool m_visible;

    Widget *                             m_parent;
    std::vector<std::unique_ptr<Widget>> m_children;

    friend class Packer;
};

#endif
