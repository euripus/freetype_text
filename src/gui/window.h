#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "widget.h"
#include "packer.h"

class UIWindow
{
public:
    UIWindow(std::string caption) : m_caption(std::move(caption)), m_visible(true) {}

    void draw();
    void update(float time);

    void        setCaption(std::string caption) { m_caption = std::move(caption); }
    std::string getCaption() const { return m_caption; }

    void show();
    void hide();
    bool visible() const;
	
	void      move(glm::vec2 const & point);
    void      resize(glm::vec2 const & new_size);

    void loadWindowDesc(std::string_view file_name);

private:
    std::string m_caption;
    bool        m_visible      = false;
    bool        m_draw_caption = false;

    std::unique_ptr<Widget> m_root;
    std::unique_ptr<Widget> m_background;
    Packer                  m_packer;
    TexFont *               m_font = nullptr;   // caption font
	glm::vec2               m_pos = {};
    glm::vec2               m_size = {};        // Window size without caption

    friend class Packer;
};

#endif
