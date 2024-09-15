#ifndef TEXT_FITTER_H
#define TEXT_FITTER_H

#include <vector>
#include <string>
#include "basic_types.h"
#include "rect2d.h"
#include "texfont.h"

namespace TextFitter
{
using Lines = std::vector<std::string>;

float       MaxStringWidthInLines(TexFont const & font, Lines const & lines);
std::string TrimWordToWidth(TexFont const & font, float const width, std::string const & word);
Lines AdjustTextToRect(TexFont const & font, Rect2D const & rect, bool stretch, std::string const & text);
}   // namespace TextFitter

#endif
