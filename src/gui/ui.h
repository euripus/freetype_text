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

    static std::unique_ptr<Widget> CreateWidget(ElementType type);

    // InputBackend * input;

    glm::vec2 m_size;   // screen size

    UIImageManager m_ui_atlas;

    std::vector<std::unique_ptr<UIWindow>> m_layers;
};

#endif
