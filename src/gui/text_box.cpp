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
	// S = wh
	// fixed_width   h = S/w
	// fixed_height  w = S/h
	// fixed_area
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

	float const text_height = (m_lines.size() + 1) * (m_font->getHeight() + m_font->getLineGap());	
	m_rect.m_extent = std::vec2(size.x, glm::max(size.y, text_height));
	m_owner.childResized();  // resize text area
}
