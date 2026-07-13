#pragma once

#include <initializer_list>
#include <string>

namespace countdown::test {

// Encodes a sequence of Unicode codepoints as a UTF-8 std::string. Test-only
// helper: it lets tests spell out words like "café" or "sœur" as codepoint
// values (e.g. utf8({'c', 'a', 'f', 0xE9})) instead of embedding literal
// accented characters in the source - literal multi-byte UTF-8 source
// characters depend on the compiler correctly detecting the source file's
// encoding, which MSVC does not do reliably without extra flags this
// project doesn't set (see alphabet.cpp's own equivalent comment).
[[nodiscard]] inline std::string utf8(std::initializer_list<char32_t> codepoints) {
    std::string result;
    for (const char32_t cp : codepoints) {
        if (cp <= 0x7F) {
            result.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return result;
}

}  // namespace countdown::test
