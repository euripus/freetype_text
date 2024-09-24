#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "basic_types.h"
#include "rect2d.h"
#include "texfont.h"
#include "uiimagemanager.h"

class UIWindow;
class VertexBuffer;
class WidgetDesc;

class Widget
{
private:
    virtual void subClassDraw(VertexBuffer & background, VertexBuffer & text) const {}
    virtual void subClassUpdate(float time, bool check_cursor) {}

public:
    Widget(WidgetDesc const & desc, UIWindow & owner);
    virtual ~Widget() = default;

    void draw(VertexBuffer & background, VertexBuffer & text) const;
    void update(float time, bool check_cursor);
    void move(glm::vec2 const & new_origin);

    // virtual & final - to prevent overriding in descendants
    virtual void addWidget(std::unique_ptr<Widget> widget) final;
    virtual void removeWidget(Widget * widget) final;
    virtual bool isChild(Widget * parent_widget) final;

    Widget * parent() const { return m_parent; }
    int32_t  getNumChildren() const { return m_children.size(); }
    Widget * getChild(int32_t num) const { return m_children[num].get(); }
    Widget * getWidgetFromIDName(std::string const & id_name);   // recursive search in the child tree

    void show() { m_visible = true; }
    void hide() { m_visible = false; }
    bool visible() const { return m_visible; }
    bool focused() const { return m_focused; }
    void sizeUpdated();

    glm::vec2   minimumSize() const { return m_min_size; }
    void        setMinimumSize(glm::vec2 size) { m_min_size = size; }
    glm::vec2   maximumSize() const { return m_max_size; }
    float       getStretch() const { return m_stretch; }
    void        setStretch(float stretch) { m_stretch = stretch; }
    glm::vec2   getSize() const { return m_rect.m_size; }
    Rect2D      getRect() const { return m_rect; }
    void        setRect(Rect2D const & rect) { m_rect = rect; }
    void        setSize(float width, float height) { m_rect.m_size = {width, height}; }
    std::string getId() const { return m_id; }
    glm::vec2   pos() const { return m_pos; }

    ElementType  getType() const { return m_type; }
    auto const & getChildren() const { return m_children; }
    auto &       getChildren() { return m_children; }
    Align        getHorizontalAlign() const { return m_horizontal; }
    Align        getVerticalAlign() const { return m_vertical; }

protected:
    float getHorizontalOffset(std::string const & line) const;

    UIWindow & m_owner;

    glm::vec2   m_min_size    = {};
    glm::vec2   m_max_size    = {};
    Rect2D      m_rect        = {};
    glm::vec2   m_pos         = {};                     // draw position
    glm::vec4   m_fields      = {1.f, 1.f, 1.f, 1.f};   // left, right, bottom, top
    std::string m_id          = {};
    std::string m_region_name = {};

    bool        m_visible               = true;
    bool        m_focused               = false;
    float       m_stretch               = 0.f;
    Align       m_horizontal            = Align::left;
    Align       m_vertical              = Align::top;
    Align       m_text_horizontal_align = Align::left;
    ElementType m_type                  = ElementType::Unknown;

    TexFont *                     m_font       = nullptr;
    RegionDataOfUITexture const * m_region_ptr = nullptr;

    Widget *                             m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;

    friend class UIWindow;
};

#endif
