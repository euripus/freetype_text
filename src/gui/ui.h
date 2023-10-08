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

    std::unique_ptr<Widget> CreateWidget(ElementType type);
    UIWindow &              loadWindow(std::string_view file_name);

    // InputBackend * input;

    glm::vec2      m_screen_size;
    UIImageManager m_ui_image_atlas;
    FontManager    m_fonts;

    std::vector<std::unique_ptr<UIWindow>> m_windows;
    std::vector<std::vector<UIWindow *>>   m_layers;
};

#endif
