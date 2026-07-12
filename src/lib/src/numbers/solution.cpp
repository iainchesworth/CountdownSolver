#include <countdown/numbers/solution.hpp>

#include <format>
#include <ranges>

namespace countdown::numbers {

std::string Solution::describe() const {
    std::string out;
    // std::views::enumerate pairs each step with its 0-based position, so the
    // step number can be printed without a manual counter.
    for (const auto [index, step] : std::views::enumerate(steps_)) {
        out += std::format("{}. {} {} {} = {}\n",
                           index + 1,
                           step.lhs,
                           symbol(step.op),
                           step.rhs,
                           step.result);
    }
    return out;
}

}  // namespace countdown::numbers
