#include "text_fitter.h"

namespace TextFitter
{
float MaxStringWidthInLines(TexFont const & font, Lines const & lines)
{
    float result = 0.f;

    for(auto const & line: lines)
    {
        result = glm::max(result, font.getTextSize(line.c_str()).x);
    }
    
    return result;
}

// boost::split() analogue
static Lines split_string(std::string const & s, char delim)
{
    Lines result;

    std::stringstream ss(s);
    std::string       item;
    while(getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

static void SplitTextForWidth(Lines & result, Lines const & words, TexFont const & font, float width, float max_height = 0.f,
                       bool trim = false)
{
    auto const  blank_width    = font.getTextSize(" ").x;
    auto const  string_height  = font.getHeight() + font.getLineGap();
    std::string current_string = {};
    float       current_width  = 0.f;
    float       current_height = string_height;

    for(auto const & word: words)
    {
        float const word_width = font.getTextSize(word.c_str()).x;

        current_width += word_width;
        if(current_width > width)
        {
            current_width = word_width;
            result.push_back(std::move(current_string));

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
            current_width  += blank_width;
        }
    }
}

Lines AdjustTextToRect(TexFont const & font, Rect2D const & rect, SizePolicy scale_mode, std::string text)
{
    Lines result;   
    auto const string_width = font.getTextSize(text.c_str()).x;

    if(string_width <= rect.m_extent.x)   // text size is smaller than area size
    {
        result.push_back(text);
    }
    else
    {
        auto const words = split_string(text, ' ');

        switch(m_scale)
        {
            case SizePolicy::scale:
            {
                float const k = rect.m_extent.y / rect.m_extent.x; // maintaining the specified proportions
                float const text_area = string_width * (font.getHeight() + font.getLineGap());
                float const width  = glm::sqrt(text_area / k);

                SplitTextForWidth(result, words, font, width);

                break;
            }
            case SizePolicy::fixed_width:
            {
                SplitTextForWidth(result, words, font, rect.m_extent.x);

                break;
            }
            case SizePolicy::fixed_height:
            {
                float const text_area = string_width * (font.getHeight() + font.getLineGap());
                float const width  =  text_area / rect.m_extent.y;

                SplitTextForWidth(result, words, font, width);

                break;
            }
            case SizePolicy::trim:
            case SizePolicy::none:
            {
                SplitTextForWidth(result, words, font, rect.m_extent.x, rect.m_extent.y, true);

                break;
            }
        }
    }

    return result;
}
}