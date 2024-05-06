#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

enum class ElementType
{
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
    scale,   // scalable
    fixed_width,
    fixed_height,
    trim,   // fixed size
    none
};

#endif
