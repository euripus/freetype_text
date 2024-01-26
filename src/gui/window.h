#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"

class UI;
class UIImageGroup;

class UIWindow
{
public:
    UIWindow(UI & owner, std::string const & image_group);

    UI & getOwner() { return m_owner; }

    void draw(VertexBuffer & vb);
    void update(float time, bool check_cursor);

    void        setCaption(std::string caption) { m_caption = std::move(caption); }
    std::string getCaption() const { return m_caption; }

    void show() { m_visible = true; }
    void hide() { m_visible = false; }
    bool visible() const { return m_visible; }

    void move(glm::vec2 const & new_origin);
    void childResized();
    // void resize(glm::vec2 const & new_size);

    Rect2D    getRect() const { return m_rect; }
    glm::vec2 size() const { return m_rect.m_extent; }
    glm::vec2 pos() const { return m_pos; }

    Widget * getRootWidget() const { return m_root.get(); }
    Widget * getWidgetFromID(std::string const & id_name) const;

    template<typename Ret>
    Ret * getWidgetFromID(std::string const & id_name) const
    {
        auto * ptr = getWidgetFromID(id_name);

        if(ptr != nullptr)
            return static_cast<Ret *>(ptr);

        return nullptr;
    }

    void loadWindowFromDesc(std::string const & file_name);

private:
    std::string m_caption;
    bool        m_visible      = false;
    bool        m_draw_caption = false;

    std::unique_ptr<Widget> m_root;
    std::unique_ptr<Widget> m_background;
    TexFont *               m_font = nullptr;   // caption font, default font
    Rect2D                  m_rect = {};
    glm::vec2               m_pos  = {};   // draw position

    UI &                 m_owner;
    UIImageGroup const * m_images = nullptr;

    friend class Packer;
};

#endif
