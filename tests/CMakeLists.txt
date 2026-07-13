# ---------------------------------------------------------------------------
# Test suites. Catch2 provides both the framework and CTest integration.
# ---------------------------------------------------------------------------
find_package(Catch2 3 REQUIRED)
include(Catch)

# --- Unit tests: fast, isolated, one behaviour each -----------------------
add_executable(countdown_unit_tests
    unit/operation_test.cpp
    unit/numbers_test.cpp
    unit/letters_test.cpp
    unit/version_test.cpp)

target_link_libraries(countdown_unit_tests
    PRIVATE
        countdown::solver
        countdown::warnings
        countdown::sanitizers
        Catch2::Catch2WithMain)

countdown_deploy_sanitizer_runtime(countdown_unit_tests)

catch_discover_tests(countdown_unit_tests)

# --- Integration tests: exercise several components together --------------
add_executable(countdown_integration_tests
    integration/solver_integration_test.cpp)

target_compile_definitions(countdown_integration_tests PRIVATE
    "COUNTDOWN_TEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")

target_link_libraries(countdown_integration_tests
    PRIVATE
        countdown::solver
        countdown::warnings
        countdown::sanitizers
        Catch2::Catch2WithMain)

countdown_deploy_sanitizer_runtime(countdown_integration_tests)

catch_discover_tests(countdown_integration_tests)
