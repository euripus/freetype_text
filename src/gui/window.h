#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"
#include "packer.h"

class UIWindow
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
	bool        m_draw_caption;

    std::unique_ptr<Widget>  m_root;
	std::unique_ptr<Widget>  m_background;
    Packer                   m_packer;
	TexFont *                m_font;         // caption font
};

struct Overlays  /// -> UI class
{
    std::vector<UIWindow *> m_layers;
};

#endif
