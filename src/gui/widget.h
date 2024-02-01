#ifndef WIDGET_H
#define WIDGET_H

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <boost/json.hpp>
#include "basic_types.h"
#include "rect2d.h"
#include "texfont.h"
#include "uiimagemanager.h"

class UIWindow;
class VertexBuffer;

struct WidgetDesc
{
    glm::vec2   size_hint    = {};
    ElementType type         = ElementType::Unknown;
    bool        visible      = true;
    std::string region_name  = {};
    std::string id_name      = {};
    SizePolicy  scale        = SizePolicy::scale;
    Align       horizontal   = Align::left;
    Align       vertical     = Align::top;
    std::string font_name    = {};
    std::string texture_name = {};   // texture name from gui_set
    float       size         = 0.0f;
    std::string static_text  = {};
    Align       text_hor     = Align::left;
};

class Widget
{
private:
    // json keys
    static constexpr char const * sid_size             = "size";
    static constexpr char const * sid_type             = "type";
    static constexpr char const * sid_visible          = "visible";
    static constexpr char const * sid_texture          = "texture";
    static constexpr char const * sid_region_name      = "region_name";
    static constexpr char const * sid_id_name          = "id_name";
    static constexpr char const * sid_size_policy      = "size_policy";
    static constexpr char const * sid_align_horizontal = "align_horizontal";
    static constexpr char const * sid_align_vertical   = "align_vertical";
    static constexpr char const * sid_font             = "font";
    static constexpr char const * sid_font_size        = "font_size";
    static constexpr char const * sid_static_text      = "static_text";
    static constexpr char const * sid_text_horizontal  = "text_horizontal";
    static constexpr char const * sid_children         = "children";

    static ElementType GetElementTypeFromString(std::string_view name);
    static SizePolicy  GetSizePolicyFromString(std::string_view name);
    static Align       GetAlignFromString(std::string_view name);

    // std::unique_ptr<Widget> CreateNullWidget(ElementType type);

public:
    static std::unique_ptr<Widget> GetWidgetFromDesc(boost::json::object const & obj, UIWindow & owner);
    static std::unique_ptr<Widget> GetWidgetFromDesc(WidgetDesc const & desc, UIWindow & owner);

private:
    virtual void subClassDraw(VertexBuffer & background, VertexBuffer & text) const {}

public:
    Widget(WidgetDesc const & desc, UIWindow & owner);
    virtual ~Widget() = default;

    void draw(VertexBuffer & background, VertexBuffer & text) const;

    virtual void update(float time, bool check_cursor);
    virtual void move(glm::vec2 const & new_origin);

    virtual void addWidget(std::unique_ptr<Widget> widget);
    virtual void removeWidget(Widget * widget);
    virtual bool isChild(Widget * parent_widget);

    Widget * parent() const { return m_parent; }
    int32_t  getNumChildren() const { return m_children.size(); }
    Widget * getChild(int32_t num) const { return m_children[num].get(); }
    Widget * getWidgetFromIDName(std::string const & id_name);   // recursive search in the child tree

    void show() { m_visible = true; }
    void hide() { m_visible = false; }
    bool visible() const { return m_visible; }
    bool focused() const { return m_focused; }

    glm::vec2   size() const { return m_rect.m_extent; }
    glm::vec2   sizeHint() const { return m_size_hint; }
    Rect2D      getRect() const { return m_rect; }
    void        setRect(Rect2D const & rect) { m_rect = rect; }
    std::string getId() const { return m_id; }
    glm::vec2   pos() const { return m_pos; }

    ElementType getType() const { return m_type; }

protected:
    UIWindow & m_owner;

    glm::vec2   m_size_hint   = {};
    Rect2D      m_rect        = {};
    glm::vec2   m_pos         = {};   // draw position
	glm::vec4   m_fields = {1.f, 1.f, 1.f, 1.f};   // left, right, bottom, top
    std::string m_id          = {};
    std::string m_region_name = {};

    bool        m_visible    = true;
    bool        m_focused    = false;
    Align       m_horizontal = Align::left;
    Align       m_vertical   = Align::top;
    SizePolicy  m_scale      = SizePolicy::scale;
    ElementType m_type       = ElementType::Unknown;

    TexFont *                     m_font       = nullptr;
    RegionDataOfUITexture const * m_region_ptr = nullptr;

    Widget *                             m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;

    friend class Packer;
    friend class UIWindow;
};

#endif
