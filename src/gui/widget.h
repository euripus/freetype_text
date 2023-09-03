#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "basic_types.h"
#include "texfont.h"

class Widget
{
public:
    Widget()          = default;
    virtual ~Widget() = default;

    virtual void update(float time);
    virtual void draw();

    virtual void addWidget(std::unique_ptr<Widget> widget, Align align = Align::left);
    virtual void removeWidget(Widget * widget);
    virtual bool isChild(Widget * widget);

    // input update
    virtual void onCursorPos(int32_t xpos, int32_t ypos);
    virtual void onMouseButton(int32_t button_code, bool press);
    virtual void onMouseWheel(int32_t xoffset, int32_t yoffset);
    virtual void onKey(int32_t key_code, bool press);

    Widget * parent() const { return m_parent; }

    void show();
    void hide();
    bool visible() const { return m_visible; }

    glm::vec2 size() const;

    glm::vec2 pos() const;
    void      move(glm::vec2 const & point);
    void      resize(glm::vec2 const & new_size);

protected:
    glm::vec2 m_size;
    glm::vec2 m_size_min;
    glm::vec2 m_size_desired;
    glm::vec2 m_pos;

    bool         m_visible;
    Align        m_align;
    ElementState m_state;
    TexFont *    m_font;

    Widget *                             m_parent;
    std::vector<std::unique_ptr<Widget>> m_children;

    friend class Packer;
};

#endif
