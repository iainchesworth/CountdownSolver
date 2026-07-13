#include <countdown/letters/alphabet.hpp>

#include <cstddef>

// Every non-ASCII codepoint below is written as a \U0000XXXX universal
// character name (with the glyph itself in a trailing comment) rather than
// as a literal accented character in the source. Literal multi-byte UTF-8
// source characters depend on the compiler correctly detecting the source
// file's encoding to parse a `U'...'` literal as exactly one codepoint;
// MSVC in particular does not reliably do this without extra flags, so
// escapes are the only encoding-agnostic option that behaves identically
// across every compiler in this project's CI matrix (MSVC, Clang, GCC,
// AppleClang).

namespace countdown::letters {
namespace {

[[nodiscard]] constexpr FoldedLetters none() noexcept {
    return FoldedLetters{};
}

[[nodiscard]] constexpr FoldedLetters one(int index) noexcept {
    return FoldedLetters{{index, 0}, 1};
}

[[nodiscard]] constexpr FoldedLetters two(int first, int second) noexcept {
    return FoldedLetters{{first, second}, 2};
}

// a/A -> 0, ..., z/Z -> 25, or -1 for anything else. Every alphabet's fold
// function falls back to this for the plain (unaccented) Latin letters.
[[nodiscard]] constexpr int ascii_lower_index(char32_t cp) noexcept {
    if (cp >= U'a' && cp <= U'z') {
        return static_cast<int>(cp - U'a');
    }
    if (cp >= U'A' && cp <= U'Z') {
        return static_cast<int>(cp - U'A');
    }
    return -1;
}

// French: 26 slots. Every accented vowel/consonant folds to its base letter
// (French Scrabble tiles carry no accents at all); oe/ae ligatures - which
// also have no tile of their own - expand to their two-letter spelling.
[[nodiscard]] FoldedLetters fold_french(char32_t cp) noexcept {
    const int base = ascii_lower_index(cp);
    if (base >= 0) {
        return one(base);
    }
    switch (cp) {
        case U'\U000000E0':  // à
        case U'\U000000C0':  // À
        case U'\U000000E2':  // â
        case U'\U000000C2':  // Â
        case U'\U000000E1':  // á
        case U'\U000000C1':  // Á
        case U'\U000000E4':  // ä
        case U'\U000000C4':  // Ä
            return one(0);
        case U'\U000000E7':  // ç
        case U'\U000000C7':  // Ç
            return one(2);
        case U'\U000000E9':  // é
        case U'\U000000C9':  // É
        case U'\U000000E8':  // è
        case U'\U000000C8':  // È
        case U'\U000000EA':  // ê
        case U'\U000000CA':  // Ê
        case U'\U000000EB':  // ë
        case U'\U000000CB':  // Ë
            return one(4);
        case U'\U000000EE':  // î
        case U'\U000000CE':  // Î
        case U'\U000000EF':  // ï
        case U'\U000000CF':  // Ï
            return one(8);
        case U'\U000000F4':  // ô
        case U'\U000000D4':  // Ô
        case U'\U000000F6':  // ö
        case U'\U000000D6':  // Ö
            return one(14);
        case U'\U000000F9':  // ù
        case U'\U000000D9':  // Ù
        case U'\U000000FB':  // û
        case U'\U000000DB':  // Û
        case U'\U000000FC':  // ü
        case U'\U000000DC':  // Ü
            return one(20);
        case U'\U000000FF':  // ÿ
        case U'\U00000178':  // Ÿ
            return one(24);
        case U'\U00000153':  // œ
        case U'\U00000152':  // Œ
            return two(14, 4);  // o, e
        case U'\U000000E6':  // æ
        case U'\U000000C6':  // Æ
            return two(0, 4);  // a, e
        default:
            return none();
    }
}

// German: 29 slots (a-z, then A-umlaut=26, O-umlaut=27, U-umlaut=28 as
// their OWN slots - official German Scrabble tiles them separately, never
// folded to A/O/U). Eszett (ss) has no tile in the official game and
// conventionally alternates with "ss", so it expands to two slot-18 ('s')
// entries.
[[nodiscard]] FoldedLetters fold_german(char32_t cp) noexcept {
    const int base = ascii_lower_index(cp);
    if (base >= 0) {
        return one(base);
    }
    switch (cp) {
        case U'\U000000E4':  // ä
        case U'\U000000C4':  // Ä
            return one(26);
        case U'\U000000F6':  // ö
        case U'\U000000D6':  // Ö
            return one(27);
        case U'\U000000FC':  // ü
        case U'\U000000DC':  // Ü
            return one(28);
        case U'\U000000DF':  // ß
            return two(18, 18);  // ss
        default:
            return none();
    }
}

// Spanish: 27 slots (a-z, then N-tilde=26 as its own slot - never folded to
// N). Accented vowels (a-acute e-acute i-acute o-acute u-acute) and the
// u-diaeresis used in gue/gui fold to their base vowel - Spanish Scrabble
// has no separate tiles for these.
[[nodiscard]] FoldedLetters fold_spanish(char32_t cp) noexcept {
    const int base = ascii_lower_index(cp);
    if (base >= 0) {
        return one(base);
    }
    switch (cp) {
        case U'\U000000E1':  // á
        case U'\U000000C1':  // Á
            return one(0);
        case U'\U000000E9':  // é
        case U'\U000000C9':  // É
            return one(4);
        case U'\U000000ED':  // í
        case U'\U000000CD':  // Í
            return one(8);
        case U'\U000000F3':  // ó
        case U'\U000000D3':  // Ó
            return one(14);
        case U'\U000000FA':  // ú
        case U'\U000000DA':  // Ú
        case U'\U000000FC':  // ü
        case U'\U000000DC':  // Ü
            return one(20);
        case U'\U000000F1':  // ñ
        case U'\U000000D1':  // Ñ
            return one(26);
        default:
            return none();
    }
}

}  // namespace

Alphabet english_alphabet() noexcept {
    return Alphabet{
        AlphabetId::english, 26,
        +[](char32_t cp) noexcept -> FoldedLetters {
            if (cp > 0x7F) {
                return none();
            }
            const int index = letter_index(static_cast<char>(cp));
            return index < 0 ? none() : one(index);
        }};
}

Alphabet french_alphabet() noexcept {
    return Alphabet{AlphabetId::french, 26, +[](char32_t cp) noexcept { return fold_french(cp); }};
}

Alphabet german_alphabet() noexcept {
    return Alphabet{AlphabetId::german, 29, +[](char32_t cp) noexcept { return fold_german(cp); }};
}

Alphabet spanish_alphabet() noexcept {
    return Alphabet{AlphabetId::spanish, 27,
                    +[](char32_t cp) noexcept { return fold_spanish(cp); }};
}

std::vector<char32_t> decode_utf8(std::string_view text) {
    std::vector<char32_t> result;
    result.reserve(text.size());

    std::size_t i = 0;
    while (i < text.size()) {
        const auto byte_at = [&](std::size_t offset) noexcept {
            return static_cast<unsigned char>(text[i + offset]);
        };
        const unsigned char lead = byte_at(0);

        char32_t cp = 0;
        std::size_t len = 0;
        if ((lead & 0x80) == 0x00) {
            cp = lead;
            len = 1;
        } else if ((lead & 0xE0) == 0xC0) {
            cp = lead & 0x1F;
            len = 2;
        } else if ((lead & 0xF0) == 0xE0) {
            cp = lead & 0x0F;
            len = 3;
        } else if ((lead & 0xF8) == 0xF0) {
            cp = lead & 0x07;
            len = 4;
        } else {
            ++i;  // Not a valid UTF-8 leading byte - skip it and resync.
            continue;
        }

        if (i + len > text.size()) {
            ++i;  // Truncated sequence at the end of the input - skip the leading byte.
            continue;
        }

        bool valid = true;
        for (std::size_t k = 1; k < len; ++k) {
            const unsigned char continuation = byte_at(k);
            if ((continuation & 0xC0) != 0x80) {
                valid = false;
                break;
            }
            cp = (cp << 6) | (continuation & 0x3F);
        }
        if (!valid) {
            ++i;
            continue;
        }

        result.push_back(cp);
        i += len;
    }

    return result;
}

Frequencies frequencies_of(std::string_view utf8_text, const Alphabet& alphabet) {
    Frequencies counts{};
    for (const char32_t cp : decode_utf8(utf8_text)) {
        const FoldedLetters folded = alphabet.fold(cp);
        for (int i = 0; i < folded.count; ++i) {
            ++counts[static_cast<std::size_t>(folded.indices[static_cast<std::size_t>(i)])];
        }
    }
    return counts;
}

}  // namespace countdown::letters
