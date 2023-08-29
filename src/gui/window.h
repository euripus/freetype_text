#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"
#include "packer.h"
#include "texfont.h"

class Window
{
    void draw();
    void update(float time);

    void        setCaption(std::string caption) { m_caption = std::move(caption); }
    std::string getCaption() const { return m_caption; }

    void show();
    void hide();
    bool visible() const;

public:
    std::string m_caption;
    bool        m_visible;

    std::unique_ptr<Widget>  m_root;
    Packer                   m_packer;
    std::unique_ptr<TexFont> m_font;
};

struct Overlays
{
    std::vector<Window *> m_layers;
};

#endif
