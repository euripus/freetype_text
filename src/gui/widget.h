#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "basic_types.h"
#include "texfont.h"

class UI;

class Widget
{
public:
    Widget(UI & owner) : m_owner(owner) {}
    virtual ~Widget() = default;

    virtual void update(float time, bool check_cursor);
    virtual void draw();

    virtual void addWidget(std::unique_ptr<Widget> widget, Align align = Align::left);
    virtual void removeWidget(Widget * widget);
    virtual bool isChild(Widget * widget);

    Widget * parent() const { return m_parent; }

    void show();
    void hide();
    bool visible() const { return m_visible; }

    glm::vec2 size() const;

    glm::vec2 pos() const;
    void      move(glm::vec2 const & point);
    void      resize(glm::vec2 const & new_size);

protected:
	UI & m_owner;

    glm::vec2 m_size;
    glm::vec2 m_size_min;
    glm::vec2 m_size_desired;
    glm::vec2 m_pos;

    bool         m_visible;
    Align        m_align;
    ElementState m_state;
    TexFont *    m_font;

    Widget *                             m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;

    friend class Packer;
};

#endif
