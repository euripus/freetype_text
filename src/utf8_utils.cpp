#include "utf8_utils.h"

std::size_t utf8_surrogate_len(char const * character)
{
    std::size_t result = 0;
    char        test_char;

    if(!character)
        return 0;

    test_char = character[0];

    if((test_char & 0x80) == 0)
        return 1;

    while(test_char & 0x80)
    {
        test_char <<= 1;
        result++;
    }

    return result;
}

// ------------------------------------------------------------ utf8_strlen ---
size_t utf8_strlen(char const * string)
{
    char const * ptr    = string;
    size_t       result = 0;

    while(*ptr)
    {
        ptr += utf8_surrogate_len(ptr);
        result++;
    }

    return result;
}

std::uint32_t utf8_to_utf32(char const * character)
{
    std::uint32_t result = -1;

    if(!character)
    {
        return result;
    }

    if((character[0] & 0x80) == 0x0)
    {
        result = character[0];
    }

    if((character[0] & 0xC0) == 0xC0)
    {
        result = ((character[0] & 0x3F) << 6) | (character[1] & 0x3F);
    }

    if((character[0] & 0xE0) == 0xE0)
    {
        result = ((character[0] & 0x1F) << (6 + 6)) | ((character[1] & 0x3F) << 6) | (character[2] & 0x3F);
    }

    if((character[0] & 0xF0) == 0xF0)
    {
        result = ((character[0] & 0x0F) << (6 + 6 + 6)) | ((character[1] & 0x3F) << (6 + 6))
                 | ((character[2] & 0x3F) << 6) | (character[3] & 0x3F);
    }

    if((character[0] & 0xF8) == 0xF8)
    {
        result = ((character[0] & 0x07) << (6 + 6 + 6 + 6)) | ((character[1] & 0x3F) << (6 + 6 + 6))
                 | ((character[2] & 0x3F) << (6 + 6)) | ((character[3] & 0x3F) << 6) | (character[4] & 0x3F);
    }

    return result;
}