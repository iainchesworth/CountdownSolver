# ---------------------------------------------------------------------------
# countdown::solver - all game logic, no GUI, no platform code.
# ---------------------------------------------------------------------------
add_library(countdown_solver)
add_library(countdown::solver ALIAS countdown_solver)

# --- Version + git stamp ---------------------------------------------------
find_package(Git QUIET)
set(COUNTDOWN_VERSION_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/include/countdown/version.hpp.in")
set(COUNTDOWN_VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/countdown/version.hpp")

# Generate once at configure time so the header exists for the first build.
set(COUNTDOWN_VERSION "${PROJECT_VERSION}")
set(SRC "${COUNTDOWN_VERSION_TEMPLATE}")
set(DST "${COUNTDOWN_VERSION_HEADER}")
set(WORKDIR "${PROJECT_SOURCE_DIR}")
include("${PROJECT_SOURCE_DIR}/cmake/GenerateVersion.cmake")

# Re-stamp on every build so the git information tracks new commits without a
# reconfigure. configure_file only rewrites when the content changes, so this
# does not needlessly recompile dependants.
add_custom_target(countdown_version ALL
    BYPRODUCTS "${COUNTDOWN_VERSION_HEADER}"
    COMMAND "${CMAKE_COMMAND}"
        "-DCOUNTDOWN_VERSION=${PROJECT_VERSION}"
        "-DSRC=${COUNTDOWN_VERSION_TEMPLATE}"
        "-DDST=${COUNTDOWN_VERSION_HEADER}"
        "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}"
        "-DWORKDIR=${PROJECT_SOURCE_DIR}"
        -P "${PROJECT_SOURCE_DIR}/cmake/GenerateVersion.cmake"
    COMMENT "Stamping version and git information"
    VERBATIM)

target_sources(countdown_solver
    PRIVATE
        src/numbers/numbers_game.cpp
        src/numbers/solution.cpp
        src/letters/alphabet.cpp
        src/letters/dictionary.cpp
        src/version.cpp)

add_dependencies(countdown_solver countdown_version)

target_include_directories(countdown_solver
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>"
        "$<INSTALL_INTERFACE:include>")

target_compile_features(countdown_solver PUBLIC cxx_std_23)

target_link_libraries(countdown_solver PRIVATE countdown::warnings countdown::sanitizers countdown::coverage)
