#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"
#include "packer.h"

class UIWindow
{
    void draw();
    void update(float time);
	
	void onCursorPos(int32_t xpos, int32_t ypos);
    void onMouseButton(int32_t button_code, bool press);
    void onMouseWheel(int32_t xoffset, int32_t yoffset);
    void onKey(int32_t key_code, bool press);

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

#endif
