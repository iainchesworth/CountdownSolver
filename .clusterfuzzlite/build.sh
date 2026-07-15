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
cmake -S "$SRC/CountdownSolver" -B "$WORK/build" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$SRC/CountdownSolver/deps/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$CXXFLAGS $LIB_FUZZING_ENGINE" \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCOUNTDOWN_ENABLE_SANITIZERS=OFF \
  -DCOUNTDOWN_BUILD_APP=OFF \
  -DCOUNTDOWN_BUILD_TESTS=OFF \
  -DCOUNTDOWN_BUILD_FUZZERS=ON

cmake --build "$WORK/build" --target fuzz_utf8 fuzz_letters fuzz_numbers -j"$(nproc)"

cp "$WORK/build/fuzz/fuzz_utf8" "$WORK/build/fuzz/fuzz_letters" "$WORK/build/fuzz/fuzz_numbers" "$OUT/"
