#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

enum class element_state
{
	normal,
	hovered,
	focus_normal,
	focus_hovered,
	pressed,
	disabled
};
	
enum class arrange
{
	horizontal, vertical
};
///The definition of horizontal alignment
enum class align
{
	left, center, right
};

///The definition of vertical alignment
enum class align_v
{
	top, center, bottom
};
	

#endif