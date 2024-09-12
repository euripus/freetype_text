#include "text_fitter.h"
#include <sstream>

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

static void SplitTextForWidth(Lines & result, Lines const & words, TexFont const & font, float const width,
                              float const max_height = 0.f, bool trim = false)
{
    auto const  blank_width    = font.getTextSize(" ").x;
    auto const  string_height  = font.getHeight() + font.getLineGap();
    std::string current_string = {};
    float       current_width  = 0.f;
    float       current_height = string_height;

    for(auto const & word: words)
    {
        float const word_width = font.getTextSize(word.c_str()).x;

        if(word_width > width)
        {
            if(!current_string.empty())
                result.push_back(std::move(current_string));
            result.push_back(TrimWordToWidth(font, width, word));
            current_width = 0.f;
            continue;
        }

        current_width += (word_width + blank_width);
        if(current_width > width)
        {
            current_width = word_width + blank_width;
            result.push_back(std::move(current_string));
            current_string += word + ' ';

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
        }
    }

    if(!current_string.empty())
        result.push_back(current_string);
}

Lines AdjustTextToRect(TexFont const & font, Rect2D const & rect, bool stretch, std::string const & text)
{
    Lines      result;
    auto const string_width = font.getTextSize(text.c_str()).x;

    if(string_width <= rect.width())   // text size is smaller than area size
    {
        result.push_back(text);
    }
    else
    {
        auto const words = split_string(text, ' ');

        if(stretch)
        {
            float const k         = rect.width() > 0 ? rect.height() / rect.width()
                                                     : 1;   // maintaining the specified proportions
            float const text_area = string_width * (font.getHeight() + font.getLineGap());
            float const width     = glm::sqrt(text_area / k);

            SplitTextForWidth(result, words, font, width);
        }
        else
        {
            SplitTextForWidth(result, words, font, rect.width(), rect.height(), true);
        }
    }

    return result;
}

std::string TrimWordToWidth(TexFont const & font, float const width, std::string const & word)
{
    std::string result;
    auto const  ellipsis_width = font.getTextSize("...").x;
    float       cur_width      = 0.f;

    if(width < ellipsis_width)
        return result;

    for(uint32_t i = 0; i < word.size(); i += utf8_surrogate_len(word.c_str() + i))
    {
        std::uint32_t ucodepoint = utf8_to_utf32(word.c_str() + i);
        Glyph const & glyph      = font.getGlyph(ucodepoint);

        if(cur_width + ellipsis_width > width)
        {
            result += std::string("...");
            break;
        }

        cur_width += glyph.advance_x;
        result    += std::string(word.c_str() + i, word.c_str() + i + utf8_surrogate_len(word.c_str() + i));
    }

    return result;
}

}   // namespace TextFitter
