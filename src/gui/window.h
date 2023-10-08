#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"
#include "packer.h"

class UIWindow
{
    UIWindow(std::string caption) : m_caption(std::move(caption)), m_visible(true) {}

    void draw();
    void update(float time);

    void        setCaption(std::string caption) { m_caption = std::move(caption); }
    std::string getCaption() const { return m_caption; }

    void show();
    void hide();
    bool visible() const;

    void loadWindow(std::string_view file_name);

public:
    std::string m_caption;
    bool        m_visible      = false;
    bool        m_draw_caption = false;

    std::unique_ptr<Widget> m_root;
    std::unique_ptr<Widget> m_background;
    Packer                  m_packer;
    TexFont *               m_font = nullptr;   // caption font
};

#endif
