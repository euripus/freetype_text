#ifndef PACKER_H
#define PACKER_H

#include "widget.h"

class UIWindow;

class Packer
{
public:
    virtual ~Packer() = default;

    void         setSpacing(float val) { m_border = val; }
    virtual void fitWidgets(UIWindow * win, float width, float height) const = 0;

protected:
    float m_border = 1.0f;
};

class StringLayout;
class MemPool;

class ChainsPacker : public Packer
{
public:
    void fitWidgets(UIWindow * win, float width, float height) const;

protected:
    void addNewString(Widget & string_node, StringLayout * parent, MemPool & pool) const;
    void addWidgetInLayout(Widget & node, StringLayout * layout) const;
};

#endif
