#ifndef TEXT_BOX_H
#define TEXT_BOX_H

#include "widget.h"

class TextBox : public Widget
{
public:
protected:
	std::string m_text = {};
	
	bool m_formated = false;
	std::vector<std::string> m_lines;
};

#endif
