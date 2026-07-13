#include <catch2/catch_test_macros.hpp>

#include <countdown/conundrum/conundrum_game.hpp>
#include <countdown/letters/dictionary.hpp>

#include <string>
#include <vector>

using countdown::SolveError;
using namespace countdown::conundrum;
using countdown::letters::Dictionary;

TEST_CASE("ConundrumGame finds a single full anagram", "[conundrum]") {
    const auto dictionary = Dictionary::from_words({"cat", "act", "dog", "creation"});

    const auto result = ConundrumGame{dictionary}.with_letters("tac").solve();
    REQUIRE(result.has_value());
    REQUIRE(*result == std::vector<std::string>{"act", "cat"});
}

TEST_CASE("ConundrumGame reports every full anagram, alphabetically", "[conundrum]") {
    const auto dictionary = Dictionary::from_words({"cat", "act", "creation", "reaction"});

    const auto result = ConundrumGame{dictionary}.with_letters("noitcaer").solve();
    REQUIRE(result.has_value());
    REQUIRE(*result == std::vector<std::string>{"creation", "reaction"});
}

TEST_CASE("ConundrumGame reports no_solution when no full anagram exists", "[conundrum]") {
    const auto dictionary = Dictionary::from_words({"cat", "dog", "creation"});

    // "tacx" isn't spellable at all from the dictionary's letters.
    const auto no_match = ConundrumGame{dictionary}.with_letters("tacx").solve();
    REQUIRE_FALSE(no_match.has_value());
    REQUIRE(no_match.error() == SolveError::no_solution);

    // "actd" could be extended from "cat"/"dog" letters but isn't a full
    // 4-letter word in the dictionary, so it's still no_solution.
    const auto too_short = ConundrumGame{dictionary}.with_letters("acdt").solve();
    REQUIRE_FALSE(too_short.has_value());
    REQUIRE(too_short.error() == SolveError::no_solution);
}

TEST_CASE("ConundrumGame reports empty_input for an empty rack", "[conundrum]") {
    const auto dictionary = Dictionary::from_words({"cat", "dog"});

    const auto result = ConundrumGame{dictionary}.with_letters("").solve();
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == SolveError::empty_input);
}

TEST_CASE("ConundrumGame exposes the letters it was given", "[conundrum]") {
    const auto dictionary = Dictionary::from_words({"cat"});

    const ConundrumGame game = ConundrumGame{dictionary}.with_letters("tac");
    REQUIRE(game.letters() == "tac");
}
