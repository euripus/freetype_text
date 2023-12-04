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

    UIWindow * loadWindow(std::string const & widgets_filename, std::string const & caption,
                          std::string const & image_roup);
    void       loadUIImageGroup(std::string const & file_name);

    // InputBackend * input;

    glm::ivec2     m_screen_size;
    UIImageManager m_ui_image_atlas;
    FontManager    m_fonts;

    std::vector<std::unique_ptr<UIWindow>> m_windows;
    std::vector<std::vector<UIWindow *>>   m_layers;
};

#endif
