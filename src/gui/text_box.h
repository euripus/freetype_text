#ifndef TEXT_BOX_H
#define TEXT_BOX_H

#include "widget.h"

class TextBox : public Widget
{
public:
    TextBox(std::string const & text, UIWindow & owner);

    // Widget interface
    void update(float time, bool check_cursor) override;
    void draw() override;

protected:
    std::string m_text = {};

    bool                     m_formated = false;
    std::vector<std::string> m_lines;
};

#endif
