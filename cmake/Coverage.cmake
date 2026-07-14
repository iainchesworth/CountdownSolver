# ---------------------------------------------------------------------------
# Coverage.cmake
#
# Defines an INTERFACE target `countdown::coverage` that, when
# COUNTDOWN_ENABLE_COVERAGE is on, turns on gcov-style source-based coverage
# instrumentation (GCC/Clang) via --coverage. Off by default: only the
# dedicated linux-gcc-coverage preset turns it on, so normal dev/CI builds
# pay no instrumentation cost.
#
# Link it PRIVATE into every first-party target whose coverage should be
# measured (library, app core, test executables) - both the instrumentation
# (compile) and the gcov runtime (link) are needed on every executable that
# ends up containing instrumented objects, even ones (like countdownsolver)
# whose own sources aren't of direct interest, since the final link must
# resolve the gcov runtime symbols pulled in by whatever it links against.
# ---------------------------------------------------------------------------

option(COUNTDOWN_ENABLE_COVERAGE "Enable gcov/llvm-cov source coverage instrumentation" OFF)

add_library(countdown_coverage INTERFACE)
add_library(countdown::coverage ALIAS countdown_coverage)

if(COUNTDOWN_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # -fno-inline keeps line/branch attribution accurate for a Debug
        # build's already-unoptimized code; --coverage covers both -fprofile-
        # arcs and -ftest-coverage plus linking the gcov runtime.
        target_compile_options(countdown_coverage INTERFACE --coverage -fno-inline)
        target_link_options(countdown_coverage INTERFACE --coverage)
    else()
        message(WARNING
            "COUNTDOWN_ENABLE_COVERAGE is on but ${CMAKE_CXX_COMPILER_ID} is not "
            "GCC/Clang; coverage instrumentation is not supported on this "
            "compiler and will be skipped.")
    endif()
endif()
