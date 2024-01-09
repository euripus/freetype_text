#include "text_box.h"
#include "window.h"
#include <sstream>

TextBox::TextBox(std::string const & text, UIWindow & owner) : Widget(owner), m_text(text)
{
    adjustTextToLines();
}

void TextBox::update(float time, bool check_cursor) {}

void TextBox::draw() {}

void TextBox::move(glm::vec2 const & new_origin)
{
    Widget::move(new_origin);

	if(!m_formated)
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

void splitTextForWidth(std::vector<std::string> const & words, float width, float max_height, bool trim)
{
	auto const blank_width = m_font->getTextSize(" ").x;
	auto const string_height = m_font->getHeight() + m_font->getLineGap();
	std::string current_string;
	float       current_width = 0.f;
	float       current_height = string_height;

	for(auto const & word : words)
	{
		float const word_width = m_font->getTextSize(word.c_str()).x;

		current_width += word_width;
		if(current_width > width)
		{
			current_width = word_width;
			m_lines.push_back(std::move(current_string));
			
			if(trim)
			{
				current_height += string_height;
				if(current_height > max_height)
					return;
			}
		}
		else
		{
			current_string += word + ' ';
			current_width += blank_width;
		}
	}
}

void TextBox::adjustTextToLines()
{
    auto const string_width  = m_font->getTextSize(m_text.c_str()).x;
	if(string_width <= m_rect.m_extent.x) // text size is smaller than area size
	{
		m_lines.push_back(m_text);
	}
	else
	{
		auto size        = m_rect.m_extent;
		auto const words = split_string(m_text, ' ');

		switch(m_scale)
		{
			case fixed_width:
			{
				splitTextForWidth(words, size.x);

				break;
			}
			case fixed_height:
			{
				auto const text_area = string_width * (m_font->getHeight() + m_font->getLineGap());
				size.x = text_area / size.y;
				
				splitTextForWidth(words, size.x);
				
				break;
			}
			case fixed_area:
			{
				splitTextForWidth(words, size.x, size.y, true);
				
				break;
			}
		}
	}

    float const text_height = m_lines.size() * (m_font->getHeight() + m_font->getLineGap());
    m_rect.m_extent         = glm::vec2(size.x, glm::max(size.y, text_height));
	m_formated = true;

    m_owner.childResized();   // resize text area message
}
