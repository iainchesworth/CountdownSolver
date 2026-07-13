#include <catch2/catch_test_macros.hpp>

#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/frequencies.hpp>
#include <countdown/letters/letters_game.hpp>

#include <string>
#include <vector>

using countdown::SolveError;
using namespace countdown::letters;

TEST_CASE("frequencies_of counts letters and is constexpr", "[letters][frequencies]") {
    STATIC_REQUIRE(frequencies_of("aab")[0] == 2);
    STATIC_REQUIRE(frequencies_of("aab")[1] == 1);

    // At run time, non-letters are ignored rather than throwing.
    const auto counts = frequencies_of("a1 b!");
    REQUIRE(counts[0] == 1);
    REQUIRE(counts[1] == 1);
}

TEST_CASE("covers respects letter multiplicity", "[letters][frequencies]") {
    const auto have = frequencies_of("aab");
    REQUIRE(covers(have, frequencies_of("ab")));
    REQUIRE(covers(have, frequencies_of("aab")));
    REQUIRE_FALSE(covers(have, frequencies_of("aaab")));  // needs three a's
    REQUIRE_FALSE(covers(have, frequencies_of("abc")));   // no c available
}

TEST_CASE("from_words normalises and de-duplicates", "[letters][dictionary]") {
    const auto dictionary = Dictionary::from_words({"Cat", "cat", "  dog  ", "can't", ""});
    // "Cat"/"cat" collapse to one; "can't" and the blank line are discarded.
    REQUIRE(dictionary.size() == 2);
}

TEST_CASE("best_words returns the longest formable words", "[letters][solve]") {
    const auto dictionary = Dictionary::from_words(
        {"cat", "react", "creation", "reaction", "cratering", "zebra"});

    const auto result = dictionary.best_words("rateciong");
    REQUIRE(result.has_value());
    // "cratering" needs two r's; "creation" and "reaction" are the longest.
    REQUIRE(*result == std::vector<std::string>{"creation", "reaction"});
}

TEST_CASE("find_matches returns words by descending length", "[letters][solve]") {
    const auto dictionary = Dictionary::from_words(
        {"cat", "react", "creation", "reaction", "cratering", "zebra"});

    const auto all = dictionary.find_matches("rateciong", 1);
    REQUIRE(all.has_value());
    // creation/reaction (8), react (5), cat (3); longest first, alpha within.
    REQUIRE(*all == std::vector<std::string>{"creation", "reaction", "react", "cat"});

    const auto min5 = dictionary.find_matches("rateciong", 5);
    REQUIRE(min5.has_value());
    REQUIRE(*min5 == std::vector<std::string>{"creation", "reaction", "react"});
}

TEST_CASE("find_matches with min == rack size yields full anagrams", "[letters][solve]") {
    const auto dictionary = Dictionary::from_words({"tracewiden", "wateredin", "notaword"});
    // "wanderite" is 9 letters; only a true 9-letter anagram should match.
    const auto anagrams = dictionary.find_matches("wanderite", 9);
    REQUIRE(anagrams.has_value());
    REQUIRE(*anagrams == std::vector<std::string>{"wateredin"});
}

TEST_CASE("best_words reports failures without throwing", "[letters][solve]") {
    const auto dictionary = Dictionary::from_words({"cat", "dog"});

    REQUIRE(dictionary.best_words("").error() == SolveError::empty_input);
    REQUIRE(dictionary.best_words("xyz").error() == SolveError::no_solution);
}

TEST_CASE("words_of_length filters to an exact length, alphabetically", "[letters][dictionary]") {
    const auto dictionary = Dictionary::from_words({"cat", "dog", "bat", "creation", "ant"});

    const auto three_letter = dictionary.words_of_length(3);
    REQUIRE(three_letter == std::vector<std::string>{"ant", "bat", "cat", "dog"});

    REQUIRE(dictionary.words_of_length(8) == std::vector<std::string>{"creation"});
    REQUIRE(dictionary.words_of_length(20).empty());
}

TEST_CASE("sample strides through the word list", "[letters][dictionary]") {
    const auto dictionary = Dictionary::from_words(
        {"aa", "bb", "cc", "dd", "ee", "ff"});  // stored sorted

    const auto picked = dictionary.sample(2, 2);
    REQUIRE(picked == std::vector<std::string>{"aa", "cc"});
}

TEST_CASE("LettersGame offers a fluent API over a dictionary", "[letters][game]") {
    const auto dictionary = Dictionary::from_words({"creation", "reaction", "cat"});

    const auto result = LettersGame{dictionary}.with_letters("rateciong").solve();
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 2);
}
