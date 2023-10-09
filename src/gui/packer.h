#ifndef PACKER_H
#define PACKER_H

#include "widget.h"

class UIWindow;

class Packer
{
public:
    using WidgetMatrix = std::vector<std::vector<Widget *>>;

    void fitWidgets(UIWindow * win) const;
    void setHorizontalSpacing(float val) { m_horizontal_spacing = val; }
    void setVerticalSpacing(float val) { m_vertical_spacing = val; }

protected:
    WidgetMatrix getMatrixFromTree(Widget * root) const;
    void         addSubTree(WidgetMatrix & ls, Widget * root) const;
    float        getRowMaxWidth(std::vector<Widget *> const & row) const;
    float        getRowMaxHeight(std::vector<Widget *> const & row) const;
    void         adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const;

    float m_horizontal_spacing = 0.0f;
    float m_vertical_spacing   = 0.0f;
};

#endif
