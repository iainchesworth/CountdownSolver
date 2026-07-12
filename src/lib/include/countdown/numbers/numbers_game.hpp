#pragma once

#include <countdown/error.hpp>
#include <countdown/numbers/solution.hpp>

#include <span>
#include <utility>
#include <vector>

namespace countdown::numbers {

struct SolveOptions {
    // When false, the search stops as soon as an exact solution is found.
    // When true, it explores the whole space (still returning a single best
    // solution) which is useful for benchmarking and testing.
    bool exhaustive = false;
};

struct SolveOutcome {
    Solution solution;
    bool exact = false;  // true if solution.value() == target()

    [[nodiscard]] friend bool operator==(const SolveOutcome&, const SolveOutcome&) = default;
};

// Models one round of the Countdown numbers game.
//
// The builder-style setters use the C++23 "deducing this" pattern: a single
// template writes the setter once and returns the correct reference category
// for the object it was called on, so both of these compose without any
// duplicated const/non-const or lvalue/rvalue overloads:
//
//   auto outcome = NumbersGame{}
//                      .with_target(532)
//                      .with_numbers(std::array{75, 25, 3, 6, 2, 1})
//                      .solve();
class NumbersGame {
public:
    template <typename Self>
    [[nodiscard]] auto&& with_target(this Self&& self, int target) {
        self.target_ = target;
        return std::forward<Self>(self);
    }

    template <typename Self>
    [[nodiscard]] auto&& with_number(this Self&& self, int number) {
        self.numbers_.push_back(number);
        return std::forward<Self>(self);
    }

    template <typename Self>
    [[nodiscard]] auto&& with_numbers(this Self&& self, std::span<const int> numbers) {
        self.numbers_.assign(numbers.begin(), numbers.end());
        return std::forward<Self>(self);
    }

    [[nodiscard]] Result<SolveOutcome> solve(const SolveOptions& options = {}) const;

    [[nodiscard]] int target() const noexcept { return target_; }
    [[nodiscard]] const std::vector<int>& numbers() const noexcept { return numbers_; }

private:
    [[nodiscard]] Result<void> validate() const;

    int target_ = 0;
    std::vector<int> numbers_;
};

}  // namespace countdown::numbers
