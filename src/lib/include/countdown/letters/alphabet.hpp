#pragma once

#include <countdown/letters/frequencies.hpp>

#include <array>
#include <cstddef>
#include <string_view>
#include <vector>

namespace countdown::letters {

// Identifies which built-in Alphabet governs a Dictionary/LettersGame/
// ConundrumGame - kept as a plain enum (not a polymorphic interface) so the
// whole pipeline stays value-typed and trivially copyable, matching the
// rest of this library's style.
enum class AlphabetId { english, french, german, spanish };

// The result of folding one decoded Unicode codepoint down to this
// alphabet's letter slots. `count` is:
//   0 - the codepoint isn't part of this alphabet at all (digits,
//       punctuation, a foreign script) - rejected as invalid input.
//   1 - the ordinary case: one codepoint folds to one letter slot (e.g.
//       French 'é' folds to the same slot as 'e').
//   2 - a ligature/digraph that expands to two letter slots for game
//       purposes (French 'œ' -> o,e; German 'ß' -> s,s), because the
//       language's own tile set has no single tile for it.
struct FoldedLetters {
    std::array<int, 2> indices{};
    int count = 0;
};

// codepoint -> FoldedLetters, per one alphabet's rules. A plain function
// pointer (not std::function) keeps Alphabet trivially copyable/constexpr-
// constructible like the rest of this library's value types.
using FoldFn = FoldedLetters (*)(char32_t) noexcept;

// One language's letter set for the Letters/Conundrum games: how many
// distinct letter slots it uses (<= kMaxAlphabetSize) and how to fold a
// decoded Unicode codepoint down to one of those slots.
struct Alphabet {
    AlphabetId id;
    std::size_t size;
    FoldFn fold;
};

// English: the original 26 a-z/A-Z slots, ASCII-only. Delegates to the
// existing letter_index(char) for the ASCII range, so behaviour for every
// existing English word/rack is unchanged from before Alphabet existed.
[[nodiscard]] Alphabet english_alphabet() noexcept;

// French: 26 slots (a-z). Every accented Latin vowel/consonant folds to its
// base letter (French Scrabble tiles have no accented letters at all -
// words match with accents stripped); the œ/æ ligatures - which also have
// no tile of their own - expand to their two-letter spelling (oe/ae).
[[nodiscard]] Alphabet french_alphabet() noexcept;

// German: 29 slots (a-z, plus Ä/Ö/Ü as their own distinct slots 26-28).
// Official German Scrabble gives Ä/Ö/Ü their own tiles - they must NOT be
// folded to A/O/U. ß has no tile in the official game at all and
// conventionally alternates with "ss", so it expands to two 's' slots.
[[nodiscard]] Alphabet german_alphabet() noexcept;

// Spanish: 27 slots (a-z, plus Ñ as its own distinct slot 26). Ñ is a
// genuinely distinct letter in Spanish and is never folded to N (matching
// official Spanish Scrabble, which gives it its own tile). Accented vowels
// (á é í ó ú, plus the diaeresis ü used in güe/güi) fold to their base
// vowel - Spanish Scrabble has no separate tiles for these.
[[nodiscard]] Alphabet spanish_alphabet() noexcept;

// Decodes UTF-8 `text` into Unicode codepoints. Permissive: a malformed or
// truncated byte sequence is skipped one byte at a time rather than
// aborting, mirroring frequencies_of()'s existing "skip invalid input"
// runtime behaviour rather than introducing a new failure mode.
[[nodiscard]] std::vector<char32_t> decode_utf8(std::string_view text);

// Decodes `utf8_text` and folds every codepoint through `alphabet`,
// counting the resulting letter slots. Codepoints the alphabet rejects
// (FoldedLetters::count == 0) are simply skipped, same as frequencies_of()
// skips non-letters today - callers that need to distinguish "no letters
// at all" / "some input was rejected" validate the raw text themselves
// first (see LettersGame::validate()/ConundrumGame::validate()).
[[nodiscard]] Frequencies frequencies_of(std::string_view utf8_text, const Alphabet& alphabet);

}  // namespace countdown::letters
