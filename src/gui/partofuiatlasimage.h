#ifndef PARTOFUIATLASIMAGE_H
#define PARTOFUIATLASIMAGE_H

#include "widget.h"

class PartOfUIAtlasImageBox : public Widget
{
public:
    PartOfUIAtlasImageBox(WidgetDesc const & desc, UIWindow & owner);

    // Widget interface
private:
    void subClassUpdate(float time, bool check_cursor);
};

#endif   // PARTOFUIATLASIMAGE_H
