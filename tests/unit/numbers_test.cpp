#include <catch2/catch_test_macros.hpp>

#include <countdown/numbers/numbers_game.hpp>

#include <array>

using countdown::SolveError;
using namespace countdown::numbers;

TEST_CASE("validation rejects malformed rounds", "[numbers][validation]") {
    SECTION("no numbers") {
        REQUIRE(NumbersGame{}.with_target(100).solve().error() == SolveError::empty_input);
    }
    SECTION("target below range") {
        const auto game = NumbersGame{}.with_target(50).with_number(5);
        REQUIRE(game.solve().error() == SolveError::target_out_of_range);
    }
    SECTION("target above range") {
        const auto game = NumbersGame{}.with_target(1000).with_number(5);
        REQUIRE(game.solve().error() == SolveError::target_out_of_range);
    }
    SECTION("non-positive number") {
        const auto game = NumbersGame{}.with_target(100).with_number(0);
        REQUIRE(game.solve().error() == SolveError::number_out_of_range);
    }
}

TEST_CASE("an exact solution is found", "[numbers][solve]") {
    constexpr std::array numbers{100, 4};
    const auto outcome = NumbersGame{}
                             .with_target(104)
                             .with_numbers(numbers)
                             .solve();

    REQUIRE(outcome.has_value());
    REQUIRE(outcome->exact);
    REQUIRE(outcome->solution.value() == 104);
}

TEST_CASE("a multiplicative solution is found", "[numbers][solve]") {
    constexpr std::array numbers{6, 25};
    const auto outcome = NumbersGame{}.with_target(150).with_numbers(numbers).solve();

    REQUIRE(outcome.has_value());
    REQUIRE(outcome->exact);
    REQUIRE(outcome->solution.value() == 150);
    REQUIRE(outcome->solution.steps().size() == 1);
    REQUIRE(outcome->solution.steps().front().op == Op::multiply);
}

TEST_CASE("the closest value is returned when no exact solution exists", "[numbers][solve]") {
    constexpr std::array numbers{50, 6};
    const auto outcome = NumbersGame{}.with_target(100).with_numbers(numbers).solve();

    REQUIRE(outcome.has_value());
    REQUIRE_FALSE(outcome->exact);
    // 50 + 6 = 56 is the nearest reachable value to 100.
    REQUIRE(outcome->solution.value() == 56);
}

TEST_CASE("describe renders each step", "[numbers][solution]") {
    constexpr std::array numbers{6, 25};
    const auto outcome = NumbersGame{}.with_target(150).with_numbers(numbers).solve();

    REQUIRE(outcome.has_value());
    REQUIRE(outcome->solution.describe() == "1. 6 x 25 = 150\n");
}
