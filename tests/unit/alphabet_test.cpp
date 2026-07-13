#include <catch2/catch_test_macros.hpp>

#include <countdown/letters/alphabet.hpp>

#include "utf8_test_support.hpp"

#include <vector>

using namespace countdown::letters;
using countdown::test::utf8;

TEST_CASE("english_alphabet accepts only ASCII letters, unchanged from before Alphabet existed",
          "[alphabet][english]") {
    const Alphabet english = english_alphabet();
    REQUIRE(english.size == 26);

    REQUIRE(english.fold(U'a').count == 1);
    REQUIRE(english.fold(U'a').indices[0] == 0);
    REQUIRE(english.fold(U'Z').count == 1);
    REQUIRE(english.fold(U'Z').indices[0] == 25);

    // Digits, punctuation, and anything beyond ASCII are rejected.
    REQUIRE(english.fold(U'5').count == 0);
    REQUIRE(english.fold(U'\'').count == 0);
    REQUIRE(english.fold(0xE9).count == 0);  // e-acute
}

TEST_CASE("french_alphabet folds accents to their base letter", "[alphabet][french]") {
    const Alphabet french = french_alphabet();
    REQUIRE(french.size == 26);

    // e-acute/e-grave/e-circumflex/e-diaeresis all fold to the 'e' slot (4),
    // matching French Scrabble's accent-free tile set.
    for (const char32_t cp : {0xE9u, 0xE8u, 0xEAu, 0xEBu, 0xC9u, 0xC8u, 0xCAu, 0xCBu}) {
        const FoldedLetters folded = french.fold(cp);
        REQUIRE(folded.count == 1);
        REQUIRE(folded.indices[0] == 4);
    }

    // c-cedilla folds to 'c' (2).
    REQUIRE(french.fold(0xE7).count == 1);
    REQUIRE(french.fold(0xE7).indices[0] == 2);

    // Plain ASCII still works.
    REQUIRE(french.fold(U'z').count == 1);
    REQUIRE(french.fold(U'z').indices[0] == 25);
}

TEST_CASE("french_alphabet expands the oe/ae ligatures to two letters", "[alphabet][french]") {
    const Alphabet french = french_alphabet();

    // No French Scrabble tile exists for oe (0x153) or ae (0xE6) - each
    // expands to its two-letter spelling instead of a single slot.
    const FoldedLetters oe = french.fold(0x153);
    REQUIRE(oe.count == 2);
    REQUIRE(oe.indices[0] == 14);  // o
    REQUIRE(oe.indices[1] == 4);   // e

    const FoldedLetters ae = french.fold(0xE6);
    REQUIRE(ae.count == 2);
    REQUIRE(ae.indices[0] == 0);  // a
    REQUIRE(ae.indices[1] == 4);  // e
}

TEST_CASE("german_alphabet keeps A-umlaut/O-umlaut/U-umlaut distinct from A/O/U",
          "[alphabet][german]") {
    const Alphabet german = german_alphabet();
    REQUIRE(german.size == 29);

    // Official German Scrabble tiles Ä/Ö/Ü separately - they must NOT fold
    // to the same slot as A/O/U.
    const FoldedLetters a = german.fold(U'a');
    const FoldedLetters a_umlaut = german.fold(0xE4);
    REQUIRE(a.count == 1);
    REQUIRE(a_umlaut.count == 1);
    REQUIRE(a.indices[0] != a_umlaut.indices[0]);
    REQUIRE(a_umlaut.indices[0] == 26);

    REQUIRE(german.fold(0xF6).indices[0] == 27);  // o-umlaut
    REQUIRE(german.fold(0xFC).indices[0] == 28);  // u-umlaut
    // Uppercase forms map to the same slots.
    REQUIRE(german.fold(0xC4).indices[0] == 26);
    REQUIRE(german.fold(0xD6).indices[0] == 27);
    REQUIRE(german.fold(0xDC).indices[0] == 28);
}

TEST_CASE("german_alphabet expands Eszett to two 's' letters", "[alphabet][german]") {
    const Alphabet german = german_alphabet();

    const FoldedLetters eszett = german.fold(0xDF);
    REQUIRE(eszett.count == 2);
    REQUIRE(eszett.indices[0] == 18);  // s
    REQUIRE(eszett.indices[1] == 18);  // s
}

TEST_CASE("spanish_alphabet keeps N-tilde distinct from N", "[alphabet][spanish]") {
    const Alphabet spanish = spanish_alphabet();
    REQUIRE(spanish.size == 27);

    const FoldedLetters n = spanish.fold(U'n');
    const FoldedLetters n_tilde = spanish.fold(0xF1);
    REQUIRE(n.indices[0] == 13);
    REQUIRE(n_tilde.count == 1);
    REQUIRE(n_tilde.indices[0] == 26);
    REQUIRE(spanish.fold(0xD1).indices[0] == 26);  // uppercase N-tilde
}

TEST_CASE("spanish_alphabet folds accented vowels (including u-diaeresis) to their base vowel",
          "[alphabet][spanish]") {
    const Alphabet spanish = spanish_alphabet();

    REQUIRE(spanish.fold(0xE1).indices[0] == 0);   // a-acute -> a
    REQUIRE(spanish.fold(0xE9).indices[0] == 4);   // e-acute -> e
    REQUIRE(spanish.fold(0xED).indices[0] == 8);   // i-acute -> i
    REQUIRE(spanish.fold(0xF3).indices[0] == 14);  // o-acute -> o
    REQUIRE(spanish.fold(0xFA).indices[0] == 20);  // u-acute -> u
    REQUIRE(spanish.fold(0xFC).indices[0] == 20);  // u-diaeresis (gue/gui) -> u
}

TEST_CASE("every alphabet rejects codepoints outside its own script", "[alphabet]") {
    // A Cyrillic letter (Cyrillic Ya, U+042F) and a CJK ideograph
    // (U+4E2D, "middle") belong to no Latin-script alphabet.
    for (const Alphabet alphabet :
         {english_alphabet(), french_alphabet(), german_alphabet(), spanish_alphabet()}) {
        REQUIRE(alphabet.fold(0x42F).count == 0);
        REQUIRE(alphabet.fold(0x4E2D).count == 0);
        REQUIRE(alphabet.fold(U'3').count == 0);
    }
}

TEST_CASE("decode_utf8 decodes multi-byte sequences correctly", "[alphabet][utf8]") {
    // "café" as UTF-8 bytes: c, a, f, then the 2-byte encoding of e-acute.
    const std::string word = utf8({U'c', U'a', U'f', 0xE9});
    const std::vector<char32_t> codepoints = decode_utf8(word);

    REQUIRE(codepoints.size() == 4);
    REQUIRE(codepoints[0] == U'c');
    REQUIRE(codepoints[1] == U'a');
    REQUIRE(codepoints[2] == U'f');
    REQUIRE(codepoints[3] == 0xE9);
}

TEST_CASE("decode_utf8 is permissive about malformed input", "[alphabet][utf8]") {
    // A lone continuation byte (0x80) with no leading byte is skipped rather
    // than aborting the whole decode, matching frequencies_of()'s existing
    // "skip invalid input" behaviour rather than introducing a new failure
    // mode.
    std::string malformed;
    malformed.push_back('a');
    malformed.push_back(static_cast<char>(0x80));
    malformed.push_back('b');

    const std::vector<char32_t> codepoints = decode_utf8(malformed);
    REQUIRE(codepoints.size() == 2);
    REQUIRE(codepoints[0] == U'a');
    REQUIRE(codepoints[1] == U'b');
}

TEST_CASE("frequencies_of folds a whole UTF-8 word through an alphabet", "[alphabet][frequencies]") {
    const Alphabet french = french_alphabet();
    // "sœur" (sister): s, oe-ligature (expands to o+e), u, r.
    const std::string soeur = utf8({U's', 0x153, U'u', U'r'});

    const Frequencies frequency = frequencies_of(soeur, french);
    REQUIRE(letter_count(frequency) == 5);  // s, o, e, u, r - the ligature counts as two letters
    REQUIRE(frequency[18] == 1);  // s
    REQUIRE(frequency[14] == 1);  // o
    REQUIRE(frequency[4] == 1);   // e
    REQUIRE(frequency[20] == 1);  // u
    REQUIRE(frequency[17] == 1);  // r
}
