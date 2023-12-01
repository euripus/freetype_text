#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "basic_types.h"
#include "texfont.h"

class UIWindow;

class Widget
{
private:
    // json keys
    static constexpr char const * sid_size             = "size";
    static constexpr char const * sid_type             = "type";
    static constexpr char const * sid_region_name      = "region_name";
    static constexpr char const * sid_id_name          = "id_name";
    static constexpr char const * sid_size_policy      = "size_policy";
    static constexpr char const * sid_align_horizontal = "align_horizontal";
    static constexpr char const * sid_align_vertical   = "align_vertical";
    static constexpr char const * sid_font             = "font";
    static constexpr char const * sid_font_size        = "font_size";

public:
    Widget(UIWindow & owner) : m_owner(owner) {}
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

    glm::vec2   size() const { return m_size; }
    glm::vec2   sizeHint() const { return m_size_hint; }
    std::string getId() const { return m_id; }

    glm::vec2 pos() const { return m_pos; }

protected:
    UIWindow & m_owner;

    glm::vec2   m_size      = {};
    glm::vec2   m_size_hint = {};
    glm::vec2   m_pos       = {};
    std::string m_id        = {};

    bool        m_visible    = true;
	bool        m_focused = false;
    Align       m_horizontal = Align::left;
    Align       m_vertical   = Align::top;
    SizePolicy  m_scale      = SizePolicy::scale;
    ElementType m_type       = ElementType::Unknown;
    TexFont *   m_font       = nullptr;

    Widget *                             m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;

    friend class Packer;
	friend class UIWindow;
};

#endif
