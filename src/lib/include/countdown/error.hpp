#pragma once

#include <expected>
#include <string_view>

namespace countdown {

// Every fallible operation in the library reports failure through this enum
// rather than by throwing. Combined with `Result<T>` (an alias for
// std::expected) this lets call sites compose with the C++23 monadic
// operations (and_then / transform / or_else) instead of try/catch.
enum class SolveError {
    empty_input,
    target_out_of_range,
    number_out_of_range,
    division_not_exact,
    non_positive_result,
    dictionary_not_found,
    dictionary_empty,
    invalid_letter,
    no_solution,
};

[[nodiscard]] constexpr std::string_view to_string(SolveError error) noexcept {
    switch (error) {
        case SolveError::empty_input:         return "no input was provided";
        case SolveError::target_out_of_range: return "target must be between 100 and 999";
        case SolveError::number_out_of_range: return "a supplied number is out of range";
        case SolveError::division_not_exact:  return "division did not produce a whole number";
        case SolveError::non_positive_result: return "operation produced a non-positive result";
        case SolveError::dictionary_not_found:return "the dictionary file could not be opened";
        case SolveError::dictionary_empty:    return "the dictionary contained no usable words";
        case SolveError::invalid_letter:      return "an invalid (non-alphabetic) letter was supplied";
        case SolveError::no_solution:         return "no solution could be found";
    }
    return "unknown error";
}

// The library-wide fallible result type. `Result<void>` is valid and is used
// for operations that either succeed or report a SolveError.
template <typename T>
using Result = std::expected<T, SolveError>;

}  // namespace countdown
