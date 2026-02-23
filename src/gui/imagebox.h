#ifndef IMAGEBOX_H
#define IMAGEBOX_H

#include "widget.h"

class ImageBox : public Widget
{
public:
    ImageBox(WidgetDesc const & desc, UIWindow & owner);

    // Widget interface
private:
    void subClassUpdate(float time, bool check_cursor);
};

#endif   // IMAGEBOX_H
