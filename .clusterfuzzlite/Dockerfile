# ClusterFuzzLite build image for countdown::solver's fuzz harnesses (see
# tests/fuzz/). base-builder provides the clang toolchain, libFuzzer, and
# sanitizer runtimes; build.sh wires them into this project's CMake build.
FROM gcr.io/oss-fuzz-base/base-builder:v1

# The base image's bundled CMake predates this project's CMake >=4
# requirement (see top-level CMakeLists.txt) - get a current one from PyPI
# rather than fighting apt/Ubuntu-release package availability.
RUN pip3 install --upgrade cmake ninja

COPY . $SRC/CountdownSolver
WORKDIR $SRC/CountdownSolver
COPY ./.clusterfuzzlite/build.sh $SRC/
