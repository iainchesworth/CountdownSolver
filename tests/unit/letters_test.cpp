#include <catch2/catch_test_macros.hpp>

#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/frequencies.hpp>
#include <countdown/letters/letters_game.hpp>

#include <filesystem>
#include <fstream>
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

TEST_CASE("best_words and find_matches report dictionary_empty for an empty dictionary",
          "[letters][dictionary]") {
    // Every entry is discarded by normalise() (digits, punctuation, a blank
    // line), so the dictionary ends up with zero usable words.
    const auto dictionary = Dictionary::from_words({"123", "!!!", ""});
    REQUIRE(dictionary.empty());

    REQUIRE(dictionary.best_words("cat").error() == SolveError::dictionary_empty);
    REQUIRE(dictionary.find_matches("cat").error() == SolveError::dictionary_empty);
}

TEST_CASE("load_from_file reports dictionary_empty when the file yields no usable words",
          "[letters][dictionary]") {
    const auto path =
        std::filesystem::temp_directory_path() / "countdown_empty_dictionary_test.txt";
    {
        std::ofstream out(path);
        out << "123\n!!!\n\n";
    }

    const auto dictionary = Dictionary::load_from_file(path);
    std::filesystem::remove(path);

    REQUIRE_FALSE(dictionary.has_value());
    REQUIRE(dictionary.error() == SolveError::dictionary_empty);
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

TEST_CASE("LettersGame reports invalid_letter for a non-alphabetic rack", "[letters][game]") {
    const auto dictionary = Dictionary::from_words({"cat", "dog"});

    const auto digit = LettersGame{dictionary}.with_letters("ca7").solve();
    REQUIRE_FALSE(digit.has_value());
    REQUIRE(digit.error() == SolveError::invalid_letter);

    const auto punctuation = LettersGame{dictionary}.add_letter('c').add_letter('\'').solve();
    REQUIRE_FALSE(punctuation.has_value());
    REQUIRE(punctuation.error() == SolveError::invalid_letter);

    // An empty rack is still reported as empty_input, not invalid_letter.
    const auto empty = LettersGame{dictionary}.with_letters("").solve();
    REQUIRE_FALSE(empty.has_value());
    REQUIRE(empty.error() == SolveError::empty_input);
}
