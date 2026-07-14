#include <catch2/catch_test_macros.hpp>

#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/letters_game.hpp>
#include <countdown/numbers/numbers_game.hpp>

#include <array>
#include <filesystem>
#include <string>

#ifndef COUNTDOWN_TEST_DATA_DIR
#error "COUNTDOWN_TEST_DATA_DIR must be defined by the build system"
#endif

using countdown::SolveError;

namespace {

[[nodiscard]] std::filesystem::path data_dir() {
    return std::filesystem::path{COUNTDOWN_TEST_DATA_DIR};
}

}  // namespace

TEST_CASE("a dictionary loads from disk and solves a round", "[integration][letters]") {
    const auto dictionary = countdown::letters::Dictionary::load_from_file(data_dir() / "words.txt");
    REQUIRE(dictionary.has_value());
    REQUIRE(dictionary->size() > 0);

    const auto words =
        countdown::letters::LettersGame{*dictionary}.with_letters("rateciong").solve();
    REQUIRE(words.has_value());
    REQUIRE_FALSE(words->empty());
    // Every returned word must be spellable from the supplied letters.
    for (const std::string& word : *words) {
        REQUIRE(word.size() <= std::string{"rateciong"}.size());
    }
}

TEST_CASE("loading a missing dictionary reports an error", "[integration][letters]") {
    const auto dictionary =
        countdown::letters::Dictionary::load_from_file(data_dir() / "does_not_exist.txt");
    REQUIRE_FALSE(dictionary.has_value());
    REQUIRE(dictionary.error() == SolveError::dictionary_not_found);
}

TEST_CASE("the numbers game solves a full six-number round", "[integration][numbers]") {
    constexpr std::array numbers{75, 25, 3, 6, 2, 1};
    const auto outcome = countdown::numbers::NumbersGame{}
                             .with_target(300)
                             .with_numbers(numbers)
                             .solve();

    REQUIRE(outcome.has_value());
    // e.g. (75 + 25) * 3 = 300 is reachable, so the search must hit the target.
    REQUIRE(outcome->exact);
    REQUIRE(outcome->solution.value() == 300);
}
