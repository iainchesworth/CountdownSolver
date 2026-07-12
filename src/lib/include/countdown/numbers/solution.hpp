#pragma once

#include <countdown/numbers/operation.hpp>

#include <string>
#include <utility>
#include <vector>

namespace countdown::numbers {

// A single arithmetic step in a solution, e.g. `75 x 4 = 300`.
struct Step {
    Value lhs{};
    Op op{};
    Value rhs{};
    Value result{};

    [[nodiscard]] friend bool operator==(const Step&, const Step&) = default;
};

// An ordered sequence of steps that evaluates to `value()`.
class Solution {
public:
    Solution() = default;

    Solution(std::vector<Step> steps, Value value)
        : steps_(std::move(steps)), value_(value) {}

    [[nodiscard]] const std::vector<Step>& steps() const noexcept { return steps_; }
    [[nodiscard]] Value value() const noexcept { return value_; }

    // Renders the solution as one `lhs op rhs = result` line per step.
    [[nodiscard]] std::string describe() const;

    [[nodiscard]] friend bool operator==(const Solution&, const Solution&) = default;

private:
    std::vector<Step> steps_;
    Value value_{};
};

}  // namespace countdown::numbers
