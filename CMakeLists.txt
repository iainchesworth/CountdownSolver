# CMake 4 is required; the version range keeps policy behaviour current up to 4.3.
cmake_minimum_required(VERSION 3.28...4.3)

# ---------------------------------------------------------------------------
# Vendored vcpkg (git submodule at deps/vcpkg).
#
# The dependency provider is pinned by the submodule so builds are fully
# reproducible without a machine-wide vcpkg or a VCPKG_ROOT env var. The
# CMakePresets point CMAKE_TOOLCHAIN_FILE at deps/vcpkg/scripts/buildsystems.
# If the submodule was not checked out, initialise it here so a fresh clone
# configures without extra steps.
# ---------------------------------------------------------------------------
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/vcpkg/scripts/buildsystems/vcpkg.cmake")
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        message(STATUS "Initialising vendored vcpkg submodule (deps/vcpkg)")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" submodule update --init --depth 1 -- deps/vcpkg
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE COUNTDOWN_SUBMODULE_RESULT)
        if(NOT COUNTDOWN_SUBMODULE_RESULT EQUAL 0)
            message(FATAL_ERROR
                "Failed to initialise the vcpkg submodule. Run:\n"
                "  git submodule update --init --depth 1 deps/vcpkg")
        endif()
    endif()
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(FATAL_ERROR
        "Vendored vcpkg not found at deps/vcpkg.\n"
        "Clone with submodules (git clone --recurse-submodules ...) or run:\n"
        "  git submodule update --init --depth 1 deps/vcpkg")
endif()

project(CountdownSolver
    VERSION 0.1.0
    DESCRIPTION "A solver for the Countdown TV show numbers and letters games"
    HOMEPAGE_URL "https://github.com/iainchesworth/CountdownSolver"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Global language settings: C++23 only, no compiler extensions.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for clang-tidy / editors.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Put all runtime artefacts in one place so the Qt app can find its plugins.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(COUNTDOWN_BUILD_APP "Build the Qt6 GUI application" ON)
option(COUNTDOWN_BUILD_TESTS "Build the unit and integration test suites" ON)

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------
# The library holds ALL of the game logic and is completely GUI-free.
add_subdirectory(src/lib)

# The application is a thin Qt6 GUI layer over the library.
if(COUNTDOWN_BUILD_APP)
    add_subdirectory(src/app)
endif()

if(COUNTDOWN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "CountdownSolver ${PROJECT_VERSION}")
message(STATUS "  Build Qt app : ${COUNTDOWN_BUILD_APP}")
message(STATUS "  Build tests  : ${COUNTDOWN_BUILD_TESTS}")
