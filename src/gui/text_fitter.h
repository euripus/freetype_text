#ifndef TEXT_FITTER_H
#define TEXT_FITTER_H

#include <vector>
#include <string>

namespace TextFitter
{
using Lines = std::vector<std::string>;

float MaxStringWidthInLines(TexFont const & font, Lines const & lines);
Lines AdjustTextToRect(TexFont const & font, Rect2D const & rect, SizePolicy scale_mode, std::string text);
}

#endif