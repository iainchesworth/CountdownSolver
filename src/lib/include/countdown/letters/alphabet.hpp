#pragma once

#include <countdown/letters/frequencies.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace countdown::letters {

// Identifies which built-in Alphabet governs a Dictionary/LettersGame/
// ConundrumGame - kept as a plain enum (not a polymorphic interface) so the
// whole pipeline stays value-typed and trivially copyable, matching the
// rest of this library's style.
enum class AlphabetId { english, french, german, spanish, arabic, hebrew, yiddish };

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
// distinct letter slots it uses (<= kMaxAlphabetSize), how to fold a
// decoded Unicode codepoint down to one of those slots, and the game-rules
// data Solver::randomRack()/randomConundrum() need to draw a rack in that
// language instead of a hardcoded English one.
struct Alphabet {
    AlphabetId id;
    std::size_t size;
    FoldFn fold;

    // slot index -> canonical UTF-8 glyph for that slot (e.g. index 26 is
    // "ä" for German, "ñ" for Spanish) - a rack needs the real glyph, not a
    // bare `char` cast, once slots go past the ASCII a-z range.
    std::vector<std::string> display_letters;

    // Scrabble-style tile frequency per slot (0 for an unused slot).
    std::array<int, kMaxAlphabetSize> letter_weights{};
    // true for a vowel slot; only consulted where vowel_consonant_splits is
    // non-empty (see below).
    std::array<bool, kMaxAlphabetSize> is_vowel{};

    // Valid {vowelCount, consonantCount} pairs, each summing to rack_size.
    // Empty means "no split enforced" - randomRack() falls back to a flat
    // weighted draw of rack_size letters from every slot in letter_weights
    // regardless of is_vowel. Abjad scripts (Arabic/Hebrew/Yiddish, added in
    // a later phase) leave this empty since they don't normally write short
    // vowels, so an English-style vowel/consonant split doesn't apply.
    std::vector<std::pair<int, int>> vowel_consonant_splits;

    // Letters drawn for a Letters-game rack or a Conundrum round. 9 for
    // every language except French, whose real-world adaptation (Des
    // chiffres et des lettres) draws 10.
    std::size_t rack_size = 9;
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

// Arabic: 29 slots (28 base letters alif..ya, plus taa marbuta as its own
// distinct slot 28). No reference game exists for this adaptation - these
// rules are original design, not copied from a real show (unlike French/
// German/Spanish, which Phase 2 sourced from real broadcast/Scrabble
// precedent). Hamza-bearing alif variants (hamza-above, hamza-below, madda)
// and the standalone hamza fold to bare alif; hamza-on-waw folds to waw;
// hamza-on-yaa and alif maksura fold to yaa - all treated as the same base
// letter for word-game purposes. No vowel/consonant split: short vowels
// aren't normally written in Arabic, so `vowel_consonant_splits` is empty
// and randomRack() draws a flat weighted sample instead (see Alphabet's
// vowel_consonant_splits comment).
[[nodiscard]] Alphabet arabic_alphabet() noexcept;

// Hebrew: 22 slots (alef..tav). Final letter forms (kaf/mem/nun/pe/tsadi
// sofit) fold to their base letter - purely positional/orthographic
// variants of the same letter, not phonetically distinct, same precedent as
// French dropping accents; Dictionary keeps the original spelling separate
// from the folded Frequencies used for matching, so nothing is lost
// displaying the correct final form. Same abjad caveat as Arabic: no
// vowel/consonant split, no reference game, original design.
[[nodiscard]] Alphabet hebrew_alphabet() noexcept;

// Yiddish: built on Hebrew's same 22-letter base (same final-form folding).
// Unlike Hebrew, Yiddish writes vowels explicitly using vowel-marking
// diacritic combinations (e.g. pasekh-alef, komets-alef) - these fold away
// via decode_utf8()'s combining-mark skip, leaving just the base alef, same
// mechanism Arabic's tashkeel relies on. Yiddish's three extra digraph
// letters (tsvey-vovn, vov-yod, tsvey-yudn) expand to their two-letter
// decomposition via the existing ligature (count==2) mechanism, the same
// one French's oe/ae and German's Eszett already use, rather than adding
// dedicated slots for them.
[[nodiscard]] Alphabet yiddish_alphabet() noexcept;

// Decodes UTF-8 `text` into Unicode codepoints. Permissive: a malformed or
// truncated byte sequence is skipped one byte at a time rather than
// aborting, mirroring frequencies_of()'s existing "skip invalid input"
// runtime behaviour rather than introducing a new failure mode. Also skips
// combining-mark codepoints entirely (never emitted in the result): Arabic
// tashkeel (U+064B-U+065F, U+0670), Hebrew points/cantillation
// (U+0591-U+05C7), and the general Unicode combining-diacriticals block
// (U+0300-U+036F, a safety net for NFD-normalized text e.g. from a
// clipboard) are meant to be silently ignored during folding, not rejected
// as "invalid" (which would reject the whole word) and not counted as a
// letter of their own. This is a deliberate behavior change from when this
// function was Latin-script-only: previously any codepoint outside an
// alphabet's fold table was rejected; combining marks are now the one
// exception, dropped rather than rejected, uniformly for every consumer
// (Dictionary::normalise(), frequencies_of(), and both
// LettersGame::validate()/ConundrumGame::validate(), which call this
// function directly on rack input) since they all decode through here.
[[nodiscard]] std::vector<char32_t> decode_utf8(std::string_view text);

// Decodes `utf8_text` and folds every codepoint through `alphabet`,
// counting the resulting letter slots. Codepoints the alphabet rejects
// (FoldedLetters::count == 0) are simply skipped, same as frequencies_of()
// skips non-letters today - callers that need to distinguish "no letters
// at all" / "some input was rejected" validate the raw text themselves
// first (see LettersGame::validate()/ConundrumGame::validate()).
[[nodiscard]] Frequencies frequencies_of(std::string_view utf8_text, const Alphabet& alphabet);

}  // namespace countdown::letters
