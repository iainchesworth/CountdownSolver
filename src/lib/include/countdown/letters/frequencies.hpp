#pragma once

#include <array>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <string_view>

namespace countdown::letters {

// The most letter slots any supported Alphabet (see alphabet.hpp) needs -
// German's 29 (a-z plus distinct ä/ö/ü tiles) is the largest today, with a
// little headroom. A letter multiset sized to this: index 0 == 'a' for
// every alphabet; slots beyond a given alphabet's own size (its `size`
// field) are simply always zero, so this single fixed width serves every
// alphabet without per-language specialization of Frequencies itself.
inline constexpr std::size_t kMaxAlphabetSize = 32;
using Frequencies = std::array<int, kMaxAlphabetSize>;

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

// Total letters a Frequencies multiset represents - a word's "play length"
// for matching/scoring, deliberately not the same thing as its stored
// spelling's byte or codepoint count: once accents are folded (without
// changing letter count) or a ligature like French oe/ae expands to two
// letters, only this - the folded slot count - is the right length to
// compare against a rack or another word.
[[nodiscard]] constexpr int letter_count(const Frequencies& frequencies) noexcept {
    int total = 0;
    for (const int count : frequencies) {
        total += count;
    }
    return total;
}

}  // namespace countdown::letters
