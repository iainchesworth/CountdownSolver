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
    if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # clang-cl: CMAKE_CXX_COMPILER_ID is "Clang" (so it would otherwise hit
        # the GNU|Clang branch below), but LLVM's bundled ASan runtime on
        # Windows - distinct from MSVC's own integrated ASan used in the
        # elseif(MSVC) branch further down - cannot link against the debug
        # CRT. Forcing the release CRT to work around that breaks every
        # debug-CRT dependency this project's Debug config actually links:
        # vcpkg's Catch2 (built debug, `/failifmismatch: _ITERATOR_DEBUG_LEVEL`)
        # and Qt's official prebuilt Debug binaries (Qt6Cored.dll etc, which
        # windeployqt then can't consistently match against a release-CRT
        # exe). Fixing that properly would need a custom vcpkg triplet plus a
        # non-standard Qt build; out of scope here. So: no ASan/UBSan under
        # clang-cl. Real MSVC (the elseif(MSVC) branch) already supports ASan
        # together with the debug CRT natively, with no such conflict.
        message(STATUS
            "ASan/UBSan disabled for clang-cl: LLVM's Windows ASan runtime "
            "requires the release CRT, which is incompatible with this "
            "project's debug-CRT vcpkg/Qt dependencies. Use windows-msvc for "
            "sanitizer coverage.")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
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
            # All targets (app, unit tests, integration tests) share the same
            # CMAKE_RUNTIME_OUTPUT_DIRECTORY. A separate POST_BUILD copy per
            # target raced on CI, where independent targets link in parallel:
            # two copy commands writing the same destination DLL at once hit
            # "permission denied" on Windows. Make the copy a single shared
            # build edge instead, so ninja only ever runs it once.
            if(NOT TARGET countdown_asan_runtime_deploy)
                get_filename_component(_countdown_asan_dll_name "${_countdown_asan_dll}" NAME)
                set(_countdown_asan_dll_dest
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_countdown_asan_dll_name}")
                add_custom_command(
                    OUTPUT "${_countdown_asan_dll_dest}"
                    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                        "${_countdown_asan_dll}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    DEPENDS "${_countdown_asan_dll}"
                    COMMENT "Copying ASan runtime into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
                add_custom_target(countdown_asan_runtime_deploy
                    DEPENDS "${_countdown_asan_dll_dest}")
                unset(_countdown_asan_dll_name)
                unset(_countdown_asan_dll_dest)
            endif()
            add_dependencies(${target} countdown_asan_runtime_deploy)
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
