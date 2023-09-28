#ifndef PACKER_H
#define PACKER_H

#include "widget.h"

class Packer
{
public:
	using FinalList = std::vector<std::vector<Widget *>>;

    void fitWidgets(Widget * root);
    void setSpacing(float val) { m_spacing = val; }

protected:
	FinalList getListFromTree(Widget * root);
	void addSubTree(FinalList & ls, Widget * root);
	
    float m_spacing;
};

#endif
