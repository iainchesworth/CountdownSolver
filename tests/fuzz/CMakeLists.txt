# ---------------------------------------------------------------------------
# libFuzzer harnesses for countdown::solver.
#
# Built only when COUNTDOWN_BUILD_FUZZERS is ON (see top-level CMakeLists.txt)
# - not part of the default build, and not run via CTest. Driven instead by
# ClusterFuzzLite (.clusterfuzzlite/build.sh), which configures this whole
# project with $CFLAGS/$CXXFLAGS/$LIB_FUZZING_ENGINE already folded into
# CMAKE_C_FLAGS/CMAKE_CXX_FLAGS/CMAKE_EXE_LINKER_FLAGS, so every translation
# unit - including countdown_solver itself, not just the harnesses below -
# gets the same sanitizer/coverage instrumentation.
# ---------------------------------------------------------------------------

# Outside ClusterFuzzLite/OSS-Fuzz (a local dev build), $LIB_FUZZING_ENGINE
# isn't set, so compile+link libFuzzer directly instead - that way
# `cmake --preset ... -DCOUNTDOWN_BUILD_FUZZERS=ON` still produces runnable,
# coverage-guided fuzzers without needing .clusterfuzzlite/build.sh at all.
# This only instruments the harness's own translation unit, not
# countdown_solver's - under ClusterFuzzLite, build.sh instead folds
# $CXXFLAGS into CMAKE_CXX_FLAGS globally, so every TU (library included)
# gets the same coverage/sanitizer instrumentation; that's the path that
# matters for real bug-finding and for the OpenSSF Scorecard Fuzzing check.
if(NOT DEFINED ENV{LIB_FUZZING_ENGINE})
    set(COUNTDOWN_FUZZER_ENGINE_FLAGS -fsanitize=fuzzer)
endif()

function(countdown_add_fuzzer name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE countdown::solver)
    target_compile_options(${name} PRIVATE ${COUNTDOWN_FUZZER_ENGINE_FLAGS})
    target_link_options(${name} PRIVATE ${COUNTDOWN_FUZZER_ENGINE_FLAGS})
    # Fuzz binaries are picked up by .clusterfuzzlite/build.sh's $OUT copy
    # step or run directly for local development - keep them out of the
    # shared app/CLI runtime output directory.
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/fuzz")
endfunction()

# Exercises the permissive UTF-8 decoder directly on arbitrary bytes - the
# lowest-level piece of untrusted-input handling in the library, and the one
# every rack/word validation path in Letters/Conundrum funnels through.
countdown_add_fuzzer(fuzz_utf8 fuzz_utf8.cpp)

# Exercises rack validation + word-matching (LettersGame::solve) against a
# small fixed in-memory dictionary, with the rack itself drawn straight from
# the fuzzer's bytes.
countdown_add_fuzzer(fuzz_letters fuzz_letters.cpp)

# Exercises the numbers-game solver's search with a fuzzer-controlled target
# and operand list.
countdown_add_fuzzer(fuzz_numbers fuzz_numbers.cpp)
