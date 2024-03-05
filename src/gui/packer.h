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
    void         addWidgetPtr(WidgetMatrix & mtx, Widget * ptr, int32_t x, int32_t y) const;
    void         addSubTree(WidgetMatrix & ls, Widget * root, int32_t x, int32_t y) const;
    float        getRowSumWidth(std::vector<Widget *> const & row) const;
    float        getSumOfFixedWidthInRow(std::vector<Widget *> const & row) const;
    int32_t      getNumOfScaledElementsInRow(std::vector<Widget *> const & row) const;
    // bool         isFixedElementInRow(std::vector<Widget *> const & row) const;
    float getRowMaxHeight(std::vector<Widget *> const & row) const;
    void  adjustWidgetsInRow(UIWindow * win, WidgetMatrix & ls, float new_width) const;

    float m_horizontal_spacing = 1.0f;
    float m_vertical_spacing   = 1.0f;
};

#endif
