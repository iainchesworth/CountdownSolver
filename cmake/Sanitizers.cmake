# ---------------------------------------------------------------------------
# Sanitizers.cmake
#
# Defines an INTERFACE target `countdown::sanitizers` that, for Debug builds,
# turns on AddressSanitizer + UndefinedBehaviorSanitizer (GCC/Clang) or
# AddressSanitizer (MSVC), plus standard-library hardening assertions. All flags
# are guarded by $<CONFIG:Debug>, so Release builds are completely unaffected.
#
# Link it PRIVATE into every first-party target (library, app, tests) so both
# the instrumentation (compile) and the runtime (link) are applied.
# ---------------------------------------------------------------------------

option(COUNTDOWN_ENABLE_SANITIZERS "Enable ASan/UBSan for Debug builds" ON)

add_library(countdown_sanitizers INTERFACE)
add_library(countdown::sanitizers ALIAS countdown_sanitizers)

if(COUNTDOWN_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(COUNTDOWN_SANITIZER_FLAGS
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-sanitize-recover=all)
        target_compile_options(countdown_sanitizers INTERFACE
            "$<$<CONFIG:Debug>:${COUNTDOWN_SANITIZER_FLAGS}>")
        target_link_options(countdown_sanitizers INTERFACE
            "$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
    elseif(MSVC)
        # MSVC ships AddressSanitizer; UBSan is not available.
        target_compile_options(countdown_sanitizers INTERFACE
            "$<$<CONFIG:Debug>:/fsanitize=address>")
        # MSVC ASan turns on STL container-overflow annotations (string, vector,
        # optional, ...), which require EVERY linked library (Qt, Catch2, ...) to
        # also be ASan-instrumented, or the link fails with LNK2038 'annotate_*'
        # mismatches. vcpkg builds its dependencies without ASan, so disable the
        # STL annotations wholesale to match.
        target_compile_definitions(countdown_sanitizers INTERFACE
            "$<$<CONFIG:Debug>:_DISABLE_STL_ANNOTATION>")
        # /RTC (runtime checks) and incremental linking are incompatible with
        # ASan, so remove /RTC from the Debug flags and disable incremental link.
        string(REGEX REPLACE "/RTC[1csu]+" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}"
            CACHE STRING "Flags used by the CXX compiler during Debug builds." FORCE)
        target_link_options(countdown_sanitizers INTERFACE
            "$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
    endif()
endif()

# Debug hardening: turn on libstdc++/libc++ precondition assertions. Harmless
# where the standard library does not recognise the macro (e.g. MSVC STL).
target_compile_definitions(countdown_sanitizers INTERFACE
    "$<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>>:_GLIBCXX_ASSERTIONS>")
