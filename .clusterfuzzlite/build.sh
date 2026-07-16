#!/bin/bash -eu
#
# Builds countdown::solver's fuzz harnesses (tests/fuzz/) for ClusterFuzzLite.
# Only the harnesses need building here - not the Qt app (no Qt available in
# this image, and none of its code is fuzzed) or the Catch2-based test suite
# (an unrelated vcpkg manifest feature). deps/vcpkg is bootstrapped
# automatically by the top-level CMakeLists.txt's own submodule-init logic,
# the same as every other build of this project.
#
# $CFLAGS/$CXXFLAGS carry ClusterFuzzLite's sanitizer/coverage instrumentation
# and must reach every translation unit, not just the harnesses - passed as
# CMAKE_C_FLAGS/CMAKE_CXX_FLAGS (global) rather than per-target, so
# countdown_solver's own sources get instrumented too. $LIB_FUZZING_ENGINE is
# appended to the *linker* flags rather than passed to
# tests/fuzz/CMakeLists.txt's own local-dev fallback, which only activates
# when $LIB_FUZZING_ENGINE is unset (see that file).
#
# CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY works around a real
# CMake+libFuzzer conflict: CMAKE_EXE_LINKER_FLAGS applies to every
# executable CMake links, including its own internal compiler-works probe
# during project() - and with -fsanitize=fuzzer in those flags, that probe
# pulls in libclang_rt.fuzzer.a, whose own main() collides with the probe's
# trivial main() ("multiple definition of main"), failing compiler detection
# outright before any real target is even configured. Building a static
# library instead for that one internal check sidesteps main() entirely.
#
# CMAKE_CXX_SCAN_FOR_MODULES=OFF: this project targets C++23 (>=20), so
# CMake's Ninja generator auto-enables C++20 modules dependency scanning by
# default (policy CMP0155) even though nothing here uses modules - and that
# scanning needs a clang-scan-deps binary CMake can't locate next to the
# custom-built clang++ in this image ("CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS-
# NOTFOUND"), failing every translation unit before compilation even starts.
# Nothing in this codebase declares/imports a module, so disabling the scan
# is a no-op for correctness.
cmake -S "$SRC/CountdownSolver" -B "$WORK/build" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$SRC/CountdownSolver/deps/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$CXXFLAGS $LIB_FUZZING_ENGINE" \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCMAKE_CXX_SCAN_FOR_MODULES=OFF \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCOUNTDOWN_ENABLE_SANITIZERS=OFF \
  -DCOUNTDOWN_BUILD_APP=OFF \
  -DCOUNTDOWN_BUILD_TESTS=OFF \
  -DCOUNTDOWN_BUILD_FUZZERS=ON

cmake --build "$WORK/build" --target fuzz_utf8 fuzz_letters fuzz_numbers -j"$(nproc)"

cp "$WORK/build/fuzz/fuzz_utf8" "$WORK/build/fuzz/fuzz_letters" "$WORK/build/fuzz/fuzz_numbers" "$OUT/"
