#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include "../widget.h"

class ImageBox : public Widget
{
public:
    ImageBox(WidgetDesc const & desc, UIWindow & owner);

private:
    void subClassFillTexturedQuads(
        std::vector<std::pair<VertexBuffer, Texture *>> & textured_quads) const override;
    void subClassUpdate(float time, bool check_cursor) override;
};

#endif   // IMAGEBOX_H
