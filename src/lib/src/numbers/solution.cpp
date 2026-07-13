#include <countdown/numbers/solution.hpp>

#include <countdown/detail/ranges_compat.hpp>

#include <format>

namespace countdown::numbers {

std::string Solution::describe() const {
    std::string out;
    // enumerate pairs each step with its 0-based position, so the step number
    // can be printed without a manual counter.
    for (const auto [index, step] : countdown::detail::enumerate(steps_)) {
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
