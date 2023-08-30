#ifndef PACKER_H
#define PACKER_H

#include "widget.h"

class Packer
{
public:
    void fitWidgets(Widget * root);
    void setSpacing(float val) { m_spacing = val; }

protected:
    float m_spacing;
};

#endif
