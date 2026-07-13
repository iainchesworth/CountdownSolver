#pragma once

#include <array>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <string_view>

namespace countdown::letters {

// A letter multiset for the 26 lowercase ASCII letters: index 0 == 'a'.
using Frequencies = std::array<int, 26>;

// Maps a character to 0..25, or -1 for anything that is not an ASCII letter.
[[nodiscard]] constexpr int letter_index(char c) noexcept {
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    return -1;
}

// Counts the letters in `text`.
//
// The `if consteval` branch gives this one function two behaviours from a
// single definition:
//   * evaluated at compile time, an invalid character makes the call a
//     non-constant expression (the throw is never a constant expression), so
//     a malformed literal is caught by the compiler; while
//   * evaluated at run time, stray punctuation or digits are simply ignored.
[[nodiscard]] constexpr Frequencies frequencies_of(std::string_view text) {
    Frequencies counts{};
    for (const char c : text) {
        const int index = letter_index(c);
        if (index < 0) {
            if consteval {
                throw std::invalid_argument("frequencies_of: non-letter in compile-time text");
            } else {
                continue;
            }
        }
        ++counts[static_cast<std::size_t>(index)];
    }
    return counts;
}

// True when every letter required by `need` is available in `have`.
// Uses std::views::zip to walk both multisets in lock-step instead of an
// index-based loop.
[[nodiscard]] constexpr bool covers(const Frequencies& have, const Frequencies& need) noexcept {
    for (const auto [available, required] : std::views::zip(have, need)) {
        if (required > available) {
            return false;
        }
    }
    return true;
}

}  // namespace countdown::letters
