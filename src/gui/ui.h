#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>
#include "src/VertexBuffer.h"
#include "fontmanager.h"
#include "uiimagemanager.h"
#include "window.h"

class UI
{
public:
    void update(float time);
    void draw(VertexBuffer & vb);

    UIWindow * loadWindow(std::string const & widgets_filename, uint32_t layer = 0,
                          std::string const & image_group = std::string("default"));
    void       parseUIResources(std::string const & file_name);

    // InputBackend * input;

    glm::ivec2     m_screen_size;
    UIImageManager m_ui_image_atlas;
    FontManager    m_fonts;
	Packer         m_packer;

    std::vector<std::unique_ptr<UIWindow>> m_windows;
    std::vector<std::vector<UIWindow *>>   m_layers;
};

#endif
