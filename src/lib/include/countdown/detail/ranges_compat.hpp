#pragma once

#include <ranges>
#include <tuple>
#include <utility>
#include <version>

// Compatibility shims for two C++23 range adaptors that libstdc++ and the MSVC
// STL implement but libc++ does not yet (as of LLVM 21 / Clang 22). Where the
// standard view exists we forward straight to it; otherwise we build an
// equivalent from adaptors libc++ *does* have (zip / iota / filter / transform).
//
// Remove these shims and inline std::views::enumerate / std::views::stride once
// libc++ ships them (expected around the Clang 23 timeframe).
//
// Define COUNTDOWN_FORCE_RANGES_COMPAT to exercise the fallback path on a
// toolchain that already has the standard views (used by the tests).

namespace countdown::detail {

#if defined(__cpp_lib_ranges_enumerate) && !defined(COUNTDOWN_FORCE_RANGES_COMPAT)

template <std::ranges::viewable_range R>
[[nodiscard]] constexpr auto enumerate(R&& range) {
    return std::views::enumerate(std::forward<R>(range));
}

#else

// Yields (index, element) like std::views::enumerate. The index type matches
// enumerate's (the range's difference type).
template <std::ranges::viewable_range R>
[[nodiscard]] constexpr auto enumerate(R&& range) {
    using diff = std::ranges::range_difference_t<R>;
    return std::views::zip(std::views::iota(diff{0}), std::forward<R>(range));
}

#endif

#if defined(__cpp_lib_ranges_stride) && !defined(COUNTDOWN_FORCE_RANGES_COMPAT)

template <std::ranges::viewable_range R>
[[nodiscard]] constexpr auto stride(R&& range, std::ranges::range_difference_t<R> step) {
    return std::views::stride(std::forward<R>(range), step);
}

#else

// Keeps every step-th element like std::views::stride: pair each element with
// its index, keep the indices divisible by step, then drop the index.
template <std::ranges::viewable_range R>
[[nodiscard]] constexpr auto stride(R&& range, std::ranges::range_difference_t<R> step) {
    using diff = std::ranges::range_difference_t<R>;
    return std::views::zip(std::views::iota(diff{0}), std::forward<R>(range))
         | std::views::filter([step](const auto& indexed) { return std::get<0>(indexed) % step == 0; })
         | std::views::transform([](auto&& indexed) -> decltype(auto) {
               return std::get<1>(std::forward<decltype(indexed)>(indexed));
           });
}

#endif

}  // namespace countdown::detail
