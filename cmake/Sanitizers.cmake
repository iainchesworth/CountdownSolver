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

# ---------------------------------------------------------------------------
# The ASan runtime (clang_rt.asan_dynamic-*.dll) isn't in a system directory,
# so executables linked against countdown::sanitizers fail to start with
# STATUS_DLL_NOT_FOUND unless run from a Developer Command Prompt. Copy it
# next to the target so it runs standalone, matching what windeployqt does
# for Qt's own DLLs.
#
# Its location differs by toolchain:
#   - real MSVC (cl.exe): next to the compiler, in the VC toolset's bin dir.
#   - clang-cl: under Clang's resource dir (`clang-cl -print-resource-dir`),
#     in lib/windows/ — NOT next to clang-cl.exe itself.
# ---------------------------------------------------------------------------
function(countdown_deploy_sanitizer_runtime target)
    if(COUNTDOWN_ENABLE_SANITIZERS AND MSVC)
        set(_countdown_asan_search_dirs "")

        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            execute_process(
                COMMAND "${CMAKE_CXX_COMPILER}" -print-resource-dir
                OUTPUT_VARIABLE _countdown_clang_resource_dir
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            if(_countdown_clang_resource_dir)
                list(APPEND _countdown_asan_search_dirs "${_countdown_clang_resource_dir}/lib/windows")
            endif()
            unset(_countdown_clang_resource_dir)
        else()
            get_filename_component(_countdown_msvc_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
            list(APPEND _countdown_asan_search_dirs "${_countdown_msvc_bin_dir}")
            unset(_countdown_msvc_bin_dir)
        endif()

        set(_countdown_asan_dll "")
        foreach(_countdown_asan_dir IN LISTS _countdown_asan_search_dirs)
            file(GLOB _countdown_asan_candidates "${_countdown_asan_dir}/clang_rt.asan_dynamic-*.dll")
            if(_countdown_asan_candidates)
                list(GET _countdown_asan_candidates 0 _countdown_asan_dll)
                break()
            endif()
        endforeach()

        if(_countdown_asan_dll)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_countdown_asan_dll}" "$<TARGET_FILE_DIR:${target}>"
                COMMENT "Copying ASan runtime next to ${target}")
        else()
            message(WARNING
                "ASan is enabled but its runtime DLL was not found (searched: "
                "${_countdown_asan_search_dirs}); ${target} may fail to start "
                "with STATUS_DLL_NOT_FOUND outside a Developer Command Prompt.")
        endif()

        unset(_countdown_asan_search_dirs)
        unset(_countdown_asan_dll)
        unset(_countdown_asan_candidates)
    endif()
endfunction()
