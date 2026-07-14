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

TEST_CASE("every alphabet's game-rules fields are internally self-consistent",
          "[alphabet][rack]") {
    for (const Alphabet& alphabet :
         {english_alphabet(), french_alphabet(), german_alphabet(), spanish_alphabet(),
          arabic_alphabet(), hebrew_alphabet(), yiddish_alphabet()}) {
        // display_letters covers every slot the alphabet actually uses.
        REQUIRE(alphabet.display_letters.size() == alphabet.size);

        // Every vowel_consonant_splits pair sums to rack_size. Empty for the
        // abjad alphabets (Arabic/Hebrew/Yiddish) - see their doc comments.
        for (const auto& [vowels, consonants] : alphabet.vowel_consonant_splits) {
            REQUIRE(static_cast<std::size_t>(vowels + consonants) == alphabet.rack_size);
        }

        // Every nonzero-weight slot is within the alphabet's own size, and a
        // slot can't be both a positive weight and simultaneously outside
        // the vowel/consonant classification's reach (is_vowel is only
        // meaningful up to `size`, but shouldn't be set beyond it either).
        int total_weight = 0;
        for (std::size_t i = 0; i < kMaxAlphabetSize; ++i) {
            if (i >= alphabet.size) {
                REQUIRE(alphabet.letter_weights[i] == 0);
            } else {
                total_weight += alphabet.letter_weights[i];
            }
        }
        REQUIRE(total_weight > 0);
    }

    // French draws 10 letters (Des chiffres et des lettres); the others
    // stay at 9, matching Phase 2's research findings (and, for
    // Arabic/Hebrew/Yiddish, the lack of any reference game to justify
    // anything else - see arabic_alphabet()'s doc comment).
    REQUIRE(french_alphabet().rack_size == 10);
    REQUIRE(english_alphabet().rack_size == 9);
    REQUIRE(german_alphabet().rack_size == 9);
    REQUIRE(spanish_alphabet().rack_size == 9);
    REQUIRE(arabic_alphabet().rack_size == 9);
    REQUIRE(hebrew_alphabet().rack_size == 9);
    REQUIRE(yiddish_alphabet().rack_size == 9);

    // The abjad alphabets leave vowel_consonant_splits empty on purpose -
    // no vowel/consonant split for scripts that don't normally write short
    // vowels (randomRack() falls back to a flat weighted draw instead).
    REQUIRE(arabic_alphabet().vowel_consonant_splits.empty());
    REQUIRE(hebrew_alphabet().vowel_consonant_splits.empty());
    REQUIRE(yiddish_alphabet().vowel_consonant_splits.empty());
}

TEST_CASE("every alphabet rejects codepoints outside its own script", "[alphabet]") {
    // A Cyrillic letter (Cyrillic Ya, U+042F) and a CJK ideograph
    // (U+4E2D, "middle") belong to no alphabet in this library, Latin or
    // otherwise.
    for (const Alphabet& alphabet : {english_alphabet(), french_alphabet(), german_alphabet(),
                                      spanish_alphabet(), arabic_alphabet(), hebrew_alphabet(),
                                      yiddish_alphabet()}) {
        REQUIRE(alphabet.fold(0x42F).count == 0);
        REQUIRE(alphabet.fold(0x4E2D).count == 0);
        REQUIRE(alphabet.fold(U'3').count == 0);
    }
    // The abjad alphabets also reject plain Latin letters - they have no
    // ascii_lower_index() fallback the way french/german/spanish do.
    for (const Alphabet& alphabet : {arabic_alphabet(), hebrew_alphabet(), yiddish_alphabet()}) {
        REQUIRE(alphabet.fold(U'a').count == 0);
    }
}

TEST_CASE("arabic_alphabet folds hamza variants and alif maksura to their base letter",
          "[alphabet][arabic]") {
    const Alphabet arabic = arabic_alphabet();
    REQUIRE(arabic.size == 29);

    // Hamza-bearing alif variants and the standalone hamza all fold to bare
    // alif (slot 0) - see arabic_alphabet()'s doc comment.
    for (const char32_t cp : {0x0627u, 0x0621u, 0x0623u, 0x0625u, 0x0622u, 0x0671u}) {
        const FoldedLetters folded = arabic.fold(cp);
        REQUIRE(folded.count == 1);
        REQUIRE(folded.indices[0] == 0);
    }
    // Hamza-on-waw folds to waw (slot 26).
    REQUIRE(arabic.fold(0x0624).indices[0] == 26);
    // Hamza-on-yaa and alif maksura both fold to yaa (slot 27).
    REQUIRE(arabic.fold(0x0626).indices[0] == 27);
    REQUIRE(arabic.fold(0x0649).indices[0] == 27);
    // Taa marbuta stays its own distinct slot (28) - not folded to haa.
    REQUIRE(arabic.fold(0x0629).count == 1);
    REQUIRE(arabic.fold(0x0629).indices[0] == 28);
    REQUIRE(arabic.fold(0x0647).indices[0] == 25);  // haa itself
}

TEST_CASE("hebrew_alphabet folds final letter forms to their base letter",
          "[alphabet][hebrew]") {
    const Alphabet hebrew = hebrew_alphabet();
    REQUIRE(hebrew.size == 22);

    // kaf/mem/nun/pe/tsadi sofit (final forms) fold to the same slot as
    // their base (medial/initial) letter.
    REQUIRE(hebrew.fold(0x05DB).indices[0] == hebrew.fold(0x05DA).indices[0]);  // kaf/kaf sofit
    REQUIRE(hebrew.fold(0x05DE).indices[0] == hebrew.fold(0x05DD).indices[0]);  // mem/mem sofit
    REQUIRE(hebrew.fold(0x05E0).indices[0] == hebrew.fold(0x05DF).indices[0]);  // nun/nun sofit
    REQUIRE(hebrew.fold(0x05E4).indices[0] == hebrew.fold(0x05E3).indices[0]);  // pe/fe sofit
    REQUIRE(hebrew.fold(0x05E6).indices[0] == hebrew.fold(0x05E5).indices[0]);  // tsadi/tsadi sofit
    // Every final form still folds to exactly one letter, not rejected.
    for (const char32_t final_form : {0x05DAu, 0x05DDu, 0x05DFu, 0x05E3u, 0x05E5u}) {
        REQUIRE(hebrew.fold(final_form).count == 1);
    }
}

TEST_CASE("yiddish_alphabet shares Hebrew's base letters and final-form folding",
          "[alphabet][yiddish]") {
    const Alphabet yiddish = yiddish_alphabet();
    const Alphabet hebrew = hebrew_alphabet();
    REQUIRE(yiddish.size == hebrew.size);

    // Same base letters, same finals-fold-to-base behaviour as Hebrew.
    REQUIRE(yiddish.fold(0x05D0).indices[0] == hebrew.fold(0x05D0).indices[0]);  // alef
    REQUIRE(yiddish.fold(0x05DB).indices[0] == yiddish.fold(0x05DA).indices[0]);  // kaf/kaf sofit
}

TEST_CASE("yiddish_alphabet expands its three digraph letters to two letters",
          "[alphabet][yiddish]") {
    const Alphabet yiddish = yiddish_alphabet();

    // tsvey-vovn -> vav, vav.
    const FoldedLetters tsvey_vovn = yiddish.fold(0x05F0);
    REQUIRE(tsvey_vovn.count == 2);
    REQUIRE(tsvey_vovn.indices[0] == 5);
    REQUIRE(tsvey_vovn.indices[1] == 5);

    // vov-yod -> vav, yod.
    const FoldedLetters vov_yod = yiddish.fold(0x05F1);
    REQUIRE(vov_yod.count == 2);
    REQUIRE(vov_yod.indices[0] == 5);
    REQUIRE(vov_yod.indices[1] == 9);

    // tsvey-yudn -> yod, yod.
    const FoldedLetters tsvey_yudn = yiddish.fold(0x05F2);
    REQUIRE(tsvey_yudn.count == 2);
    REQUIRE(tsvey_yudn.indices[0] == 9);
    REQUIRE(tsvey_yudn.indices[1] == 9);

    // Hebrew itself has no special case for these ligature codepoints - it
    // isn't required to recognise them at all, only Yiddish is.
    REQUIRE(hebrew_alphabet().fold(0x05F2).count == 0);
}

TEST_CASE("decode_utf8 skips combining marks instead of emitting them",
          "[alphabet][utf8][combining]") {
    // Arabic tashkeel (fatha, U+064B) between two consonants should vanish
    // entirely from the decoded codepoint stream, not appear as its own
    // entry and not break the surrounding letters apart.
    const std::string with_tashkeel = utf8({0x0628u, 0x064Bu, 0x062Fu});  // baa, fathatan, dal
    const std::vector<char32_t> decoded = decode_utf8(with_tashkeel);
    REQUIRE(decoded.size() == 2);
    REQUIRE(decoded[0] == 0x0628);
    REQUIRE(decoded[1] == 0x062F);

    // Hebrew points (e.g. qamats, U+05B8) are skipped the same way - this is
    // what lets a Yiddish pasekh-alef/komets-alef combination fold down to
    // just the base alef (see yiddish_alphabet()'s doc comment).
    const std::string with_qamats = utf8({0x05D0u, 0x05B8u});  // alef, qamats
    REQUIRE(decode_utf8(with_qamats) == std::vector<char32_t>{0x05D0});

    // The general combining-diacriticals block (e.g. combining acute accent,
    // U+0301) is skipped too - a safety net for NFD-normalized Latin text.
    const std::string nfd_e_acute = utf8({U'e', 0x0301u});
    REQUIRE(decode_utf8(nfd_e_acute) == std::vector<char32_t>{U'e'});
}

TEST_CASE("a word containing only a combining mark alongside real letters still loads",
          "[alphabet][dictionary]") {
    // Confirms the combining-mark skip actually prevents rejection: before
    // this, any codepoint outside an alphabet's fold table - including a
    // stray combining mark - would reject the entire word.
    const Alphabet arabic = arabic_alphabet();
    const std::string word_with_tashkeel = utf8({0x0628u, 0x064Bu, 0x062Fu});
    const Frequencies frequency = frequencies_of(word_with_tashkeel, arabic);
    REQUIRE(letter_count(frequency) == 2);  // baa + dal; the fatha contributes nothing
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
