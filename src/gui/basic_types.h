#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

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

enum class SizePolicy
{
    scale,
    fixed_width,
    fixed_height,
    fixed_area,
    none
};

#endif
