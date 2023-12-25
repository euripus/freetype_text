#include "text_box.h"
#include <sstream>

TextBox::TextBox(std::string const & text, UIWindow & owner) : Widget(owner), m_text(text)
{
    adjustTextToLines();
}

void TextBox::update(float time, bool check_cursor) {}

void TextBox::draw() {}

void TextBox::adjustSize()
{
    adjustTextToLines();

    Widget::adjustSize();   // forward message to children
}

void TextBox::setText(std::string const & new_text)
{
    m_text     = new_text;
    m_formated = false;

    adjustTextToLines();
}

// boost::split() analogue
static std::vector<std::string> split_string(std::string const & s, char delim)
{
    std::vector<std::string> result;

    std::stringstream ss(s);
    std::string       item;
    while(getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

void TextBox::adjustTextToLines()
{
    auto const size        = m_rect.m_extent;
    auto const words       = split_string(m_text, ' ');
    auto const blank_width = m_font->getTextSize(" ").x;

    std::string current_string;
    float       current_width = 0.f;
    for(auto const & word : words)
    {
        float const word_width = m_font->getTextSize(word.c_str()).x;

        current_width += word_width;
        if(current_width > size.x)
        {
            current_width = word_width;
            m_lines.push_back(std::move(current_string));
        }
        else
        {
            current_string += word + ' ';
            current_width += blank_width;
        }
    }
}
