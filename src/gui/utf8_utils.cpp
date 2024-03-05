#include "utf8_utils.h"

// Maximum valid value for a Unicode code point
constexpr uint32_t CODE_POINT_MAX = 0x0010ffffu;
// Leading (high) surrogates: 0xd800 - 0xdbff
// Trailing (low) surrogates: 0xdc00 - 0xdfff
constexpr uint16_t LEAD_SURROGATE_MIN  = 0xd800u;
constexpr uint16_t TRAIL_SURROGATE_MAX = 0xdfffu;

constexpr bool is_surrogate(uint16_t cp)
{
    return (cp >= LEAD_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
}

constexpr bool is_code_point_valid(uint32_t cp)
{
    return (cp <= CODE_POINT_MAX && !is_surrogate(static_cast<uint16_t>(cp)));
}

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

std::string utf32_to_utf8(std::uint32_t cp)
{
    std::string result;

    if(!is_code_point_valid(cp))
    {
        return result;
    }

    if(cp < 0x80)   // one octet
        result += static_cast<char>(cp);
    else if(cp < 0x800)
    {   // two octets
        result += static_cast<char>((cp >> 6) | 0xc0);
        result += static_cast<char>((cp & 0x3f) | 0x80);
    }
    else if(cp < 0x10000)
    {   // three octets
        result += static_cast<char>((cp >> 12) | 0xe0);
        result += static_cast<char>(((cp >> 6) & 0x3f) | 0x80);
        result += static_cast<char>((cp & 0x3f) | 0x80);
    }
    else
    {   // four octets
        result += static_cast<char>((cp >> 18) | 0xf0);
        result += static_cast<char>(((cp >> 12) & 0x3f) | 0x80);
        result += static_cast<char>(((cp >> 6) & 0x3f) | 0x80);
        result += static_cast<char>((cp & 0x3f) | 0x80);
    }

    return result;
}
