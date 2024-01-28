#ifndef TEXT_BOX_H
#define TEXT_BOX_H

#include "widget.h"

class TextBox : public Widget
{
public:
    TextBox(WidgetDesc const & desc, UIWindow & owner);

    // Widget interface
    void update(float time, bool check_cursor) override;

    void setText(std::string new_text);

private:
    void adjustTextToLines();
	void subClassDraw(VertexBuffer & background, VertexBuffer & text) const override;

protected:
    std::string m_text = {};
	Align       m_text_horizontal_align = Align::left;

    std::vector<std::string> m_lines    = {};
    bool                     m_formated = false;
};

#endif
