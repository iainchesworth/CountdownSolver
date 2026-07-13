# ---------------------------------------------------------------------------
# CompilerWarnings.cmake
#
# Defines an INTERFACE target `countdown::warnings` that turns on a strict,
# cross-compiler warning set with "warnings as errors". Link it PRIVATE-ly
# into every first-party target (library, app, tests). Third-party code (Qt,
# Catch2) is pulled in as SYSTEM headers by their package configs, so these
# flags never fire on dependency code.
# ---------------------------------------------------------------------------

add_library(countdown_warnings INTERFACE)
add_library(countdown::warnings ALIAS countdown_warnings)

# The mandated baseline is -Wall -Wextra -Werror; the remaining flags catch the
# classes of mistake (shadowing, dubious conversions, missing overrides) that a
# generator is most likely to introduce.
set(COUNTDOWN_GNU_CLANG_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wshadow
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wimplicit-fallthrough
    -Wformat=2)

set(COUNTDOWN_MSVC_WARNINGS
    /W4
    /WX
    /permissive-)

target_compile_options(countdown_warnings INTERFACE
    "$<$<CXX_COMPILER_ID:MSVC>:${COUNTDOWN_MSVC_WARNINGS}>"
    "$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:${COUNTDOWN_GNU_CLANG_WARNINGS}>")
