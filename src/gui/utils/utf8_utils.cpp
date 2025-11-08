#include "utf8_utils.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

constexpr uint32_t CODE_POINT_MAX      = 0x0010ffffu;
constexpr uint16_t LEAD_SURROGATE_MIN  = 0xd800u;
constexpr uint16_t TRAIL_SURROGATE_MAX = 0xdfffu;

constexpr bool is_surrogate(uint32_t cp)
{
    // cp might be > 0xffff; check range explicitly
    return (cp >= LEAD_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
}

constexpr bool is_code_point_valid(uint32_t cp)
{
    return (cp <= CODE_POINT_MAX && !is_surrogate(cp));
}

// Return length in bytes of the UTF-8 sequence starting at 'character'.
// Returns 0 for invalid leading byte or if sequence would be >4 bytes.
std::size_t utf8_sequence_length(char const * character)
{
    if(!character)
        return 0;

    unsigned char c = static_cast<unsigned char>(character[0]);

    if(c < 0x80u)
        return 1u;
    if((c & 0xE0u) == 0xC0u)
        return 2u;
    if((c & 0xF0u) == 0xE0u)
        return 3u;
    if((c & 0xF8u) == 0xF0u)
        return 4u;

    // 0x80..0xBF are continuation bytes (invalid as leading bytes)
    // 0xF8..0xFF are invalid in UTF-8
    return 0u;
}

// Backwards-compatible name retained, but uses safer implementation.
std::size_t utf8_surrogate_len(char const * character)
{
    return utf8_sequence_length(character);
}

size_t utf8_strlen(char const * string)
{
    if(!string)
        return 0;

    char const * ptr    = string;
    size_t       result = 0;

    while(*ptr)
    {
        std::size_t len = utf8_sequence_length(ptr);

        if(len == 0)
        {
            // invalid leading byte -> treat it as one byte to advance (avoids infinite loop)
            ++ptr;
            ++result;
            continue;
        }

        // Ensure there are enough bytes and continuation bytes are valid.
        bool valid = true;
        for(std::size_t i = 1; i < len; ++i)
        {
            unsigned char c = static_cast<unsigned char>(ptr[i]);
            if(c == '\0' || (c & 0xC0u) != 0x80u)
            {
                valid = false;
                break;
            }
        }

        if(!valid)
        {
            // malformed/short sequence: treat leading byte as one code point
            ++ptr;
            ++result;
            continue;
        }

        ptr += len;
        ++result;
    }

    return result;
}

// Returns UINT32_MAX (i.e., (uint32_t)-1) on error/invalid sequence.
std::uint32_t utf8_to_utf32(char const * character)
{
    if(!character)
        return std::numeric_limits<std::uint32_t>::max();

    unsigned char b0 = static_cast<unsigned char>(character[0]);

    // 1-byte (ASCII)
    if((b0 & 0x80u) == 0u)
    {
        return static_cast<std::uint32_t>(b0);
    }

    // 2-byte sequence: 110xxxxx 10xxxxxx
    if((b0 & 0xE0u) == 0xC0u)
    {
        unsigned char b1 = static_cast<unsigned char>(character[1]);
        if(b1 == '\0' || (b1 & 0xC0u) != 0x80u)
            return std::numeric_limits<std::uint32_t>::max();

        uint32_t cp = ((b0 & 0x1Fu) << 6) | (b1 & 0x3Fu);
        // overlong encoding check: must be >= 0x80
        if(cp < 0x80u || !is_code_point_valid(cp))
            return std::numeric_limits<std::uint32_t>::max();

        return cp;
    }

    // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
    if((b0 & 0xF0u) == 0xE0u)
    {
        unsigned char b1 = static_cast<unsigned char>(character[1]);
        unsigned char b2 = static_cast<unsigned char>(character[2]);
        if(b1 == '\0' || b2 == '\0' || (b1 & 0xC0u) != 0x80u || (b2 & 0xC0u) != 0x80u)
            return std::numeric_limits<std::uint32_t>::max();

        uint32_t cp = ((b0 & 0x0Fu) << 12) | ((b1 & 0x3Fu) << 6) | (b2 & 0x3Fu);
        // overlong: must be >= 0x800. Also disallow surrogates.
        if(cp < 0x800u || !is_code_point_valid(cp))
            return std::numeric_limits<std::uint32_t>::max();

        return cp;
    }

    // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if((b0 & 0xF8u) == 0xF0u)
    {
        unsigned char b1 = static_cast<unsigned char>(character[1]);
        unsigned char b2 = static_cast<unsigned char>(character[2]);
        unsigned char b3 = static_cast<unsigned char>(character[3]);
        if(b1 == '\0' || b2 == '\0' || b3 == '\0' || (b1 & 0xC0u) != 0x80u || (b2 & 0xC0u) != 0x80u
           || (b3 & 0xC0u) != 0x80u)
            return std::numeric_limits<std::uint32_t>::max();

        uint32_t cp = ((b0 & 0x07u) << 18) | ((b1 & 0x3Fu) << 12) | ((b2 & 0x3Fu) << 6) | (b3 & 0x3Fu);
        // must be >= 0x10000 and <= CODE_POINT_MAX
        if(cp < 0x10000u || !is_code_point_valid(cp))
            return std::numeric_limits<std::uint32_t>::max();

        return cp;
    }

    // Invalid leading byte (including continuation bytes used as leading)
    return std::numeric_limits<std::uint32_t>::max();
}

std::string utf32_to_utf8(std::uint32_t cp)
{
    std::string result;

    if(!is_code_point_valid(cp))
    {
        return result;
    }

    if(cp < 0x80u)   // one octet
    {
        result += static_cast<char>(cp);
    }
    else if(cp < 0x800u)
    {   // two octets
        result += static_cast<char>(((cp >> 6) & 0x1Fu) | 0xC0u);
        result += static_cast<char>((cp & 0x3Fu) | 0x80u);
    }
    else if(cp < 0x10000u)
    {   // three octets
        result += static_cast<char>(((cp >> 12) & 0x0Fu) | 0xE0u);
        result += static_cast<char>(((cp >> 6) & 0x3Fu) | 0x80u);
        result += static_cast<char>((cp & 0x3Fu) | 0x80u);
    }
    else
    {   // four octets
        result += static_cast<char>(((cp >> 18) & 0x07u) | 0xF0u);
        result += static_cast<char>(((cp >> 12) & 0x3Fu) | 0x80u);
        result += static_cast<char>(((cp >> 6) & 0x3Fu) | 0x80u);
        result += static_cast<char>((cp & 0x3Fu) | 0x80u);
    }

    return result;
}
