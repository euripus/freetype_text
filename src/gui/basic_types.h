#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

enum class ElementType
{
    Empty,
    TextBox,
    ImageBox,   // animated/static image and/or internal rendered frame
    Button,
    CheckBox,
    // RadioButton,
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
    scalable,
    fixed_width,    // scaled height
    fixed_height,   // scaled width
    fixed_size,
    none
};

#endif
