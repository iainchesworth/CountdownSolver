#include <catch2/catch_test_macros.hpp>

#include <countdown/numbers/operation.hpp>

using countdown::SolveError;
using namespace countdown::numbers;

TEST_CASE("apply is usable in constant expressions", "[numbers][operation]") {
    STATIC_REQUIRE(apply(Op::add, 3, 4).value() == 7);
    STATIC_REQUIRE(apply(Op::multiply, 6, 7).value() == 42);
}

TEST_CASE("subtraction must keep a positive result", "[numbers][operation]") {
    REQUIRE(apply(Op::subtract, 10, 4).value() == 6);

    REQUIRE_FALSE(apply(Op::subtract, 4, 10).has_value());
    REQUIRE(apply(Op::subtract, 4, 10).error() == SolveError::non_positive_result);

    // A zero result is also disallowed.
    REQUIRE_FALSE(apply(Op::subtract, 5, 5).has_value());
}

TEST_CASE("division must be exact and non-zero", "[numbers][operation]") {
    REQUIRE(apply(Op::divide, 12, 3).value() == 4);

    REQUIRE_FALSE(apply(Op::divide, 12, 5).has_value());
    REQUIRE(apply(Op::divide, 12, 5).error() == SolveError::division_not_exact);

    REQUIRE_FALSE(apply(Op::divide, 12, 0).has_value());
}

TEST_CASE("symbols render each operator", "[numbers][operation]") {
    REQUIRE(symbol(Op::add) == '+');
    REQUIRE(symbol(Op::subtract) == '-');
    REQUIRE(symbol(Op::multiply) == 'x');
    REQUIRE(symbol(Op::divide) == '/');
}
