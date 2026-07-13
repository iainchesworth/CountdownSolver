#include <countdown/numbers/numbers_game.hpp>

#include <countdown/detail/ranges_compat.hpp>
#include <countdown/numbers/operation.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace countdown::numbers {
namespace {

// The real game deals exactly 6 tiles (one each of 25/50/75/100, two each of
// 1-10), so 6 is both the maximum term count and the maximum individual
// value a caller can legitimately supply. Enforcing both in validate() -
// rather than only in the QML UI - closes off the search()/apply() paths
// below to callers of the public library API directly (e.g.
// Solver::solveNumbers), which would otherwise be able to trigger
// combinatorial-cost searches or push apply() towards overflow with
// arbitrarily long or large input.
constexpr std::size_t kMaxNumberCount = 6;
constexpr int kMaxNumberValue = 100;

// A live term in the search: its current value plus the steps used to build it.
struct Term {
    Value value{};
    std::vector<Step> steps;
};

[[nodiscard]] Value distance(Value value, Value target) {
    return value > target ? value - target : target - value;
}

// Records `term` as the new best if it's closer to target than whatever's
// there already. `term` is still needed afterwards (it's about to be pushed
// into the working set for recursion), so an improvement copies its steps
// rather than moving them.
void update_best(const Term& term, Value target, std::optional<SolveOutcome>& best) {
    const bool improves =
        !best || distance(term.value, target) < distance(best->solution.value(), target);
    if (improves) {
        best = SolveOutcome{Solution{term.steps, term.value}, term.value == target};
    }
}

// Depth-first search over every way of combining two terms into one. Because
// each combination shrinks the working set by one, recursion terminates. The
// closest-to-target term seen anywhere in the tree is kept in `best`.
//
// Every term already present in `terms` on entry was already checked against
// `best` by its parent call (or, for the initial call, by the caller) - only
// a newly-merged term is genuinely new information, so combine() below is the
// sole place update_best() runs, rather than rescanning the whole working set
// at every node.
void search(std::vector<Term>& terms, Value target,
            std::optional<SolveOutcome>& best, bool exhaustive) {
    if (best && best->exact && !exhaustive) {
        return;
    }

    const std::size_t count = terms.size();
    for (const std::size_t i : std::views::iota(std::size_t{0}, count)) {
        for (const std::size_t j : std::views::iota(i + 1, count)) {
            const Term& a = terms[i];
            const Term& b = terms[j];

            // Everything except the two terms being combined carries forward.
            std::vector<Term> rest;
            rest.reserve(count - 1);
            for (const auto [index, term] : countdown::detail::enumerate(terms)) {
                const auto k = static_cast<std::size_t>(index);
                if (k != i && k != j) {
                    rest.push_back(term);
                }
            }

            const auto combine = [&](Op op, const Term& x, const Term& y) {
                if (const Result<Value> result = apply(op, x.value, y.value)) {
                    Term merged{*result, x.steps};
                    merged.steps.insert(merged.steps.end(), y.steps.begin(), y.steps.end());
                    merged.steps.push_back(Step{x.value, op, y.value, *result});

                    update_best(merged, target, best);
                    rest.push_back(std::move(merged));
                    search(rest, target, best, exhaustive);
                    rest.pop_back();
                }
            };

            // Commutative operators need one ordering; the others need both.
            combine(Op::add, a, b);
            combine(Op::multiply, a, b);
            combine(Op::subtract, a, b);
            combine(Op::subtract, b, a);
            combine(Op::divide, a, b);
            combine(Op::divide, b, a);

            if (best && best->exact && !exhaustive) {
                return;
            }
        }
    }
}

}  // namespace

Result<void> NumbersGame::validate() const {
    if (numbers_.empty()) {
        return std::unexpected(SolveError::empty_input);
    }
    if (target_ < 100 || target_ > 999) {
        return std::unexpected(SolveError::target_out_of_range);
    }
    if (numbers_.size() > kMaxNumberCount) {
        return std::unexpected(SolveError::count_out_of_range);
    }
    if (std::ranges::any_of(numbers_,
                             [](int n) { return n <= 0 || n > kMaxNumberValue; })) {
        return std::unexpected(SolveError::number_out_of_range);
    }
    return {};
}

Result<SolveOutcome> NumbersGame::solve(const SolveOptions& options) const {
    // validate() first; only run the search if the round is well-formed. The
    // whole flow is expressed as a monadic chain with no exceptions.
    return validate().and_then([&]() -> Result<SolveOutcome> {
        std::vector<Term> terms;
        terms.reserve(numbers_.size());
        for (const int number : numbers_) {
            terms.push_back(Term{static_cast<Value>(number), {}});
        }

        // search() only checks newly-merged terms against `best` (see its
        // comment); the initial leaves haven't been merged from anything, so
        // they're checked once here before recursing.
        std::optional<SolveOutcome> best;
        const Value target = static_cast<Value>(target_);
        for (const Term& term : terms) {
            update_best(term, target, best);
        }
        search(terms, target, best, options.exhaustive);

        if (!best) {
            return std::unexpected(SolveError::no_solution);
        }
        return *best;
    });
}

}  // namespace countdown::numbers
