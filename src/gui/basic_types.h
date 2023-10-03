#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

enum class ElementState
{
    normal,
    hovered,
    focus_normal,
    focus_hovered,
    pressed,
    disabled
};

enum class ElementType
{
    TextBox,
    ImageBox,
    Button,
    CheckBox,
    RadioButton,
    Slider,
    ProgressBar,
    InputBox,
    ScrollView,
    VerticalLayoutee,
    HorizontalLayoutee,
	Unknown
};

enum class Align
{
    left,
    center,
    right,
    top,
    bottom
};

#endif
