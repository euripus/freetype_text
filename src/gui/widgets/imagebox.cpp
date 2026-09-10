#include "imagebox.h"

ImageBox::ImageBox(WidgetDesc const & desc, UIWindow & owner)
    : Widget(desc, owner)
{}

void ImageBox::subClassFillTexturedQuads(
    std::vector<std::pair<VertexBuffer, Texture *>> & textured_quads) const
{}

void ImageBox::subClassUpdate(float time, bool check_cursor) {}
