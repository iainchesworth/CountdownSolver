#include <catch2/catch_test_macros.hpp>

#include <countdown/conundrum/conundrum_game.hpp>
#include <countdown/letters/alphabet.hpp>
#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/letters_game.hpp>

#include "utf8_test_support.hpp"

#include <string>
#include <vector>

using countdown::SolveError;
using namespace countdown::letters;
using countdown::conundrum::ConundrumGame;
using countdown::test::utf8;

namespace {

// Small hand-written word lists exercising each alphabet's folding rules -
// real per-language dictionaries are a later phase (see the session's plan).
[[nodiscard]] std::vector<std::string> french_words() {
    return {
        "chat",                             // cat - plain ASCII
        utf8({'c', 'a', 'f', 0xE9}),         // café - has e-acute
        utf8({'s', 0x153, 'u', 'r'}),        // sœur (sister) - has the oe ligature
        utf8({'g', 'a', 'r', 0xE7, 'o', 'n'}),  // garçon (boy) - has c-cedilla
    };
}

[[nodiscard]] std::vector<std::string> german_words() {
    return {
        "haus",                                        // house - plain ASCII
        utf8({'m', 0xE4, 'd', 'c', 'h', 'e', 'n'}),     // mädchen (girl) - has a-umlaut
        utf8({'g', 'r', 0xFC, 'n'}),                    // grün (green) - has u-umlaut
        utf8({'s', 't', 'r', 'a', 0xDF, 'e'}),          // straße (street) - has Eszett
    };
}

[[nodiscard]] std::vector<std::string> spanish_words() {
    return {
        "casa",                                  // house - plain ASCII
        utf8({'n', 'i', 0xF1, 'o'}),              // niño (child) - has n-tilde
        utf8({'a', 0xF1, 'o'}),                   // año (year) - has n-tilde
        utf8({'c', 'a', 'n', 'c', 'i', 0xF3, 'n'}),  // canción (song) - has o-acute
    };
}

}  // namespace

TEST_CASE("a French dictionary matches an unaccented rack against an accented word",
          "[i18n][french][dictionary]") {
    const auto dictionary = Dictionary::from_words(french_words(), french_alphabet());

    // "café" folds to c/a/f/e - a plain ASCII rack with exactly those
    // letters should find it, displayed with its original accent.
    const auto result = dictionary.best_words("aefc");
    REQUIRE(result.has_value());
    REQUIRE(*result == std::vector<std::string>{utf8({'c', 'a', 'f', 0xE9})});
}

TEST_CASE("word length reflects folded letters, not UTF-8 byte count", "[i18n][french][dictionary]") {
    const auto dictionary = Dictionary::from_words(french_words(), french_alphabet());
    const std::string cafe = utf8({'c', 'a', 'f', 0xE9});

    // "café" is 5 bytes in UTF-8 (c,a,f, then a 2-byte e-acute) but 4
    // letters - it must be filed under length 4 (alongside the plain-ASCII
    // "chat", also 4 letters), not 5.
    REQUIRE(dictionary.words_of_length(4) == std::vector<std::string>{cafe, "chat"});
}

TEST_CASE("the oe ligature expands to two letters for length/matching purposes",
          "[i18n][french][dictionary]") {
    const auto dictionary = Dictionary::from_words(french_words(), french_alphabet());
    const std::string soeur = utf8({'s', 0x153, 'u', 'r'});

    // "sœur" is 4 codepoints (s, oe-ligature, u, r) but plays as 5 letters
    // (s, o, e, u, r) - no French Scrabble tile has the ligature itself.
    REQUIRE(dictionary.words_of_length(5) == std::vector<std::string>{soeur});

    const auto found = dictionary.best_words("soeur");  // plain ASCII rack, 5 letters
    REQUIRE(found.has_value());
    REQUIRE(*found == std::vector<std::string>{soeur});
}

TEST_CASE("a German dictionary keeps a-umlaut distinct from plain a", "[i18n][german][dictionary]") {
    const auto dictionary = Dictionary::from_words(german_words(), german_alphabet());
    const std::string maedchen = utf8({'m', 0xE4, 'd', 'c', 'h', 'e', 'n'});

    // A rack using the real a-umlaut finds "mädchen".
    const auto with_umlaut = dictionary.best_words(utf8({'m', 0xE4, 'd', 'c', 'h', 'e', 'n'}));
    REQUIRE(with_umlaut.has_value());
    REQUIRE(*with_umlaut == std::vector<std::string>{maedchen});

    // The same letters but with plain 'a' instead of a-umlaut must NOT
    // match - German Scrabble tiles Ä separately from A, so this rack is
    // missing a required letter (a-umlaut) and has an extra, useless one (a).
    const auto with_plain_a = dictionary.best_words("madchen");
    REQUIRE_FALSE(with_plain_a.has_value());
    REQUIRE(with_plain_a.error() == SolveError::no_solution);
}

TEST_CASE("Eszett expands to two 's' letters for a German dictionary", "[i18n][german][dictionary]") {
    const auto dictionary = Dictionary::from_words(german_words(), german_alphabet());
    const std::string maedchen = utf8({'m', 0xE4, 'd', 'c', 'h', 'e', 'n'});
    const std::string strasse_eszett = utf8({'s', 't', 'r', 'a', 0xDF, 'e'});

    // "straße" folds to s,t,r,a,s,s,e - 7 letters, even though it's only 6
    // codepoints (the Eszett is one codepoint that plays as two letters).
    // "mädchen" is also (coincidentally) 7 folded letters, so both appear
    // here, alphabetically.
    REQUIRE(dictionary.words_of_length(7) == std::vector<std::string>{maedchen, strasse_eszett});

    // Solving specifically for the "strasse" (plain ASCII "ss") rack still
    // isolates just the Eszett word, since "mädchen"'s letters don't cover it.
    const auto found = dictionary.best_words("strasse");
    REQUIRE(found.has_value());
    REQUIRE(*found == std::vector<std::string>{strasse_eszett});
}

TEST_CASE("a Spanish dictionary keeps n-tilde distinct from n", "[i18n][spanish][dictionary]") {
    const auto dictionary = Dictionary::from_words(spanish_words(), spanish_alphabet());
    const std::string nino = utf8({'n', 'i', 0xF1, 'o'});

    const auto with_tilde = dictionary.best_words(nino);
    REQUIRE(with_tilde.has_value());
    REQUIRE(*with_tilde == std::vector<std::string>{nino});

    // Substituting plain 'n' for n-tilde must not match "niño" - Spanish
    // Scrabble tiles Ñ separately from N.
    const auto with_plain_n = dictionary.best_words("nino");
    REQUIRE_FALSE(with_plain_n.has_value());
}

TEST_CASE("a Spanish dictionary folds accented vowels to their base vowel",
          "[i18n][spanish][dictionary]") {
    const auto dictionary = Dictionary::from_words(spanish_words(), spanish_alphabet());
    const std::string cancion = utf8({'c', 'a', 'n', 'c', 'i', 0xF3, 'n'});

    // A plain-ASCII rack (o instead of o-acute) still finds "canción",
    // because accented vowels fold to their base vowel in Spanish.
    const auto result = dictionary.best_words("cancion");
    REQUIRE(result.has_value());
    REQUIRE(*result == std::vector<std::string>{cancion});
}

TEST_CASE("LettersGame validates a French rack against the dictionary's own alphabet",
          "[i18n][french][game]") {
    const auto dictionary = Dictionary::from_words(french_words(), french_alphabet());

    // A rack containing a real accented codepoint (e-acute) is accepted,
    // not rejected as invalid_letter.
    const auto accented = LettersGame{dictionary}.with_letters(utf8({'c', 'a', 'f', 0xE9})).solve();
    REQUIRE(accented.has_value());

    // A rack containing a Cyrillic letter is still rejected.
    const auto foreign_script =
        LettersGame{dictionary}.with_letters(utf8({'c', 'a', 't', 0x42F})).solve();
    REQUIRE_FALSE(foreign_script.has_value());
    REQUIRE(foreign_script.error() == SolveError::invalid_letter);
}

TEST_CASE("ConundrumGame finds a full anagram of an accented rack", "[i18n][german][game]") {
    const auto dictionary = Dictionary::from_words(german_words(), german_alphabet());

    // "grün" scrambled - same four folded letters (g, r, u-umlaut, n) in a
    // different order.
    const auto result =
        ConundrumGame{dictionary}.with_letters(utf8({0xFC, 'g', 'n', 'r'})).solve();
    REQUIRE(result.has_value());
    REQUIRE(*result == std::vector<std::string>{utf8({'g', 'r', 0xFC, 'n'})});
}

TEST_CASE("ConundrumGame's anagram length is the rack's folded length, not its byte count",
          "[i18n][german][game]") {
    const auto dictionary = Dictionary::from_words(german_words(), german_alphabet());
    const std::string strasse_eszett = utf8({'s', 't', 'r', 'a', 0xDF, 'e'});

    // "straße" folds to 7 letters (s,t,r,a,s,s,e - the Eszett expands to
    // two). A 7-letter plain-ASCII rack with that exact multiset ("ss"
    // instead of the Eszett) is a full anagram and must be found as one -
    // not rejected as the wrong length by comparing against the rack's own
    // 7-codepoint (here, coincidentally byte-equal) count instead of its
    // folded letter count.
    const auto result = ConundrumGame{dictionary}.with_letters("strasse").solve();
    REQUIRE(result.has_value());
    REQUIRE(*result == std::vector<std::string>{strasse_eszett});
}
