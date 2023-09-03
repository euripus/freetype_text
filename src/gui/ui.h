#ifndef GUI_H
#define GUI_H

#include <glm/glm.hpp>
#include "src/VertexBuffer.h"
#include "uiimagemanager.h"
#include "window.h"

class UI
{
public:
    void update(float time);
    void draw(VertexBuffer & vb);

    // input update
    void onCursorPos(int32_t xpos, int32_t ypos);
    void onMouseButton(int32_t button_code, bool press);
    void onMouseWheel(int32_t xoffset, int32_t yoffset);
    void onKey(int32_t key_code, bool press);

    static std::unique_ptr<Widget> CreateWidget(ElementType type);

    glm::vec2 m_size;   // screen size

    glm::vec2 m_old_mouse;      // old mouse coordinates
    glm::vec2 m_mouse_dx;       // mouse movement
    int       m_mouse_button;   // mouse button

    UIImageManager m_ui_atlas;

    std::vector<std::unique_ptr<UIWindow>> m_layers;
};

#endif
