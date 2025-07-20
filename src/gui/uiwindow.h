#ifndef UIWINDOW_H
#define UIWINDOW_H

#include <string>
#include <functional>
#include "widget.h"

class UI;
class UIImageGroup;

class UIWindow
{
public:
    UIWindow(UI & owner, std::string const & image_group);

    UI &                 getOwner() { return m_owner; }
    bool                 isImageGroupExist() const { return m_images != nullptr; }
    UIImageGroup const & getImageGroup() const { return *m_images; }

    void draw(VertexBuffer & background, VertexBuffer & text) const;
    void update(float time, bool check_cursor);

    void        setCaption(std::string caption) { m_caption = std::move(caption); }
    std::string getCaption() const { return m_caption; }

    void show() { m_visible = true; }
    void hide() { m_visible = false; }
    bool visible() const { return m_visible; }
    void sizeUpdated() { m_size_updated = true; }

    void move(glm::vec2 const & new_origin);

    Rect2D    getRect() const { return m_rect; }
    void      setRect(Rect2D const & rect) { m_rect = rect; }
    void      setSize(float width, float height) { m_rect.m_size = {width, height}; }
    float     getSpacing() const { return m_spacing; }
    glm::vec2 size() const { return m_rect.m_size; }
    glm::vec2 pos() const { return m_pos; }

    Widget * getRootWidget() const;
    Widget * getBackgroundWidget() const;
    Widget * getWidgetFromID(std::string const & id_name) const;

    template<typename Ret>
    Ret * getWidgetFromID(std::string const & id_name) const
    {
        auto * ptr = getWidgetFromID(id_name);

        if(ptr != nullptr)
            return static_cast<Ret *>(ptr);

        return nullptr;
    }

    void addCallBack(std::function<void(void)> fn);

private:
    std::string m_caption;
    float       m_spacing      = 0.f;
    bool        m_visible      = false;
    bool        m_draw_caption = false;
    bool        m_size_updated = true;

    std::unique_ptr<Widget> m_root;
    std::unique_ptr<Widget> m_background;
    TexFont *               m_font = nullptr;   // caption font, default font
    Rect2D                  m_rect = {};
    glm::vec2               m_pos  = {};   // draw position

    UI &                 m_owner;
    UIImageGroup const * m_images = nullptr;

    std::vector<std::function<void(void)>> m_callbacks;

    friend struct WindowDesc;
};

#endif
