#ifndef UTF8_UTILS_H
#define UTF8_UTILS_H

#include <cstddef>
#include <cstdint>

std::size_t   utf8_surrogate_len(char const * character);
size_t        utf8_strlen(char const * string);
std::uint32_t utf8_to_utf32(char const * character);

#endif   // UTF8_UTILS_H
