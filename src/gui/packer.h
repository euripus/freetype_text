#ifndef PACKER_H
#define PACKER_H

#include "widget.h"

class Packer
{
public:
    using FinalList = std::vector<std::vector<Widget *>>;

    void fitWidgets(UIWindow * win) const;
    void setSpacing(float val) { m_spacing = val; }

protected:
    FinalList getListFromTree(Widget * root) const;
    void      addSubTree(FinalList & ls, Widget * root) const;
	float     getRowMaxWidth(std::vector<Widget *> const & row) const;
	float     getRowMaxHeight(std::vector<Widget *> const & row) const;
	void      adjustWidgetsInRow(UIWindow * win, FinalList & ls, float new_width) const;

    float m_horizontal_spacing = 0.0f;
	float m_vertical_spacing = 0.0f;
};

#endif
