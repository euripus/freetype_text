#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"
#include "packer.h"

class UI;
class UIImageGroup;

class UIWindow
{
public:
    UIWindow(UI & owner, std::string caption, std::string_view image_group);

    void draw();
    void update(float time, bool check_cursor);

    void        setCaption(std::string caption) { m_caption = std::move(caption); }
    std::string getCaption() const { return m_caption; }

    void show();
    void hide();
    bool visible() const;

    void move(glm::vec2 const & point);
    void resize(glm::vec2 const & new_size);

    Rect2D    getRect() const { return m_rect; }
    glm::vec2 size() const { return m_rect.m_extent; }
    glm::vec2 pos() const { return m_rect.m_pos; }

    void loadWindowFromDesc(std::string const & file_name);

private:
    std::unique_ptr<Widget> CreateNullWidget();

    std::string m_caption;
    bool        m_visible      = false;
    bool        m_draw_caption = false;

    std::unique_ptr<Widget> m_root;
    std::unique_ptr<Widget> m_background;
    Packer                  m_packer;
    TexFont *               m_font = nullptr;   // caption font, default font
    Rect2D                  m_rect = {};
	glm::vec2 m_pos = {}; // draw position 

    UI &                 m_owner;
    UIImageGroup const * m_images = nullptr;

    friend class Packer;
};

#endif
