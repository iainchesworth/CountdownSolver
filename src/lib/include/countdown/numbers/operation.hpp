#pragma once

#include <countdown/error.hpp>

#include <cstdint>

namespace countdown::numbers {

// Countdown intermediate results can grow large (e.g. 100 * 75 * 50 ...), so a
// 64-bit type is used to keep every partial product well clear of overflow.
using Value = std::int64_t;

enum class Op : std::uint8_t {
    add,
    subtract,
    multiply,
    divide,
};

[[nodiscard]] constexpr char symbol(Op op) noexcept {
    switch (op) {
        case Op::add:      return '+';
        case Op::subtract: return '-';
        case Op::multiply: return 'x';
        case Op::divide:   return '/';
    }
    return '?';
}

// Applies an operator under the rules of the Countdown numbers game:
//   * every intermediate value must stay a positive integer, and
//   * division must be exact.
// Rather than throwing on a rule violation, the failure is returned as a
// SolveError so callers can chain with std::expected's monadic operations.
[[nodiscard]] constexpr Result<Value> apply(Op op, Value lhs, Value rhs) noexcept {
    switch (op) {
        case Op::add:
            return lhs + rhs;
        case Op::subtract:
            if (lhs <= rhs) {
                return std::unexpected(SolveError::non_positive_result);
            }
            return lhs - rhs;
        case Op::multiply:
            return lhs * rhs;
        case Op::divide:
            if (rhs == 0 || lhs % rhs != 0) {
                return std::unexpected(SolveError::division_not_exact);
            }
            return lhs / rhs;
    }
    return std::unexpected(SolveError::non_positive_result);
}

}  // namespace countdown::numbers
