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
    unit/conundrum_test.cpp
    unit/version_test.cpp)

target_link_libraries(countdown_unit_tests
    PRIVATE
        countdown::solver
        countdown::warnings
        countdown::sanitizers
        countdown::coverage
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
        countdown::coverage
        Catch2::Catch2WithMain)

countdown_deploy_sanitizer_runtime(countdown_integration_tests)

catch_discover_tests(countdown_integration_tests)

# ---------------------------------------------------------------------------
# App tests: exercise the Qt-facing Solver facade (countdown_app_core) with
# Qt Test, without linking Quick/Gui/QuickControls2. Only built alongside the
# app itself, since countdown_app_core only exists when COUNTDOWN_BUILD_APP
# is on.
# ---------------------------------------------------------------------------
if(COUNTDOWN_BUILD_APP)
    # find_package-created imported targets are scoped to the directory tree
    # they were created in; src/app's own find_package(Qt6 ...) call doesn't
    # make Qt6::Quick/Gui/QuickControls2 visible in this sibling directory, so
    # every component either test target below needs is (re-)requested here.
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test QuickTest Quick Gui QuickControls2)

    # qt_standard_project_setup() (called in src/app/CMakeLists.txt) sets
    # CMAKE_AUTOMOC ON as a directory-scoped variable, which only propagates
    # to descendants of that directory - tests/ is a sibling of src/app, not
    # a descendant, so it must enable AUTOMOC for its own Qt Test targets.
    set(CMAKE_AUTOMOC ON)

    # solver_test.hpp is listed explicitly (not just #included) so AUTOMOC
    # reliably scans it for Q_OBJECT regardless of basename-matching quirks.
    qt_add_executable(countdown_app_tests app/solver_test.cpp app/solver_test.hpp)

    target_link_libraries(countdown_app_tests
        PRIVATE
            # WHOLE_ARCHIVE: countdown_app_core's bundled-dictionary resource
            # object file has no direct symbol references, so a plain link
            # would let the linker silently drop it - see the qt_add_resources
            # comment in src/app/CMakeLists.txt.
            $<LINK_LIBRARY:WHOLE_ARCHIVE,countdown_app_core>
            countdown::warnings
            countdown::sanitizers
            countdown::coverage
            Qt6::Test)

    countdown_deploy_sanitizer_runtime(countdown_app_tests)

    # windeployqt only runs against countdown_app (see src/app/CMakeLists.txt)
    # and only copies the Qt DLLs *that target* depends on, so Qt6Test.dll
    # never lands in the shared bin/ output directory otherwise: the test exe
    # fails to start (STATUS_DLL_NOT_FOUND) even though it links fine.
    if(WIN32 AND COUNTDOWN_WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET countdown_app_tests POST_BUILD
            COMMAND "${COUNTDOWN_WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --compiler-runtime
                "$<TARGET_FILE:countdown_app_tests>"
            COMMENT "Deploying Qt runtime dependencies for countdown_app_tests")
    endif()

    add_test(NAME countdown_app_tests COMMAND countdown_app_tests)

    # -----------------------------------------------------------------------
    # QML tests: Qt Quick Test drives the real "Countdown" QML module (pages +
    # solver wiring) headlessly. countdown_app_qml is the same compiled module
    # countdown_app itself links, so this exercises the exact QML shipped -
    # no separate copy of the QML_FILES list.
    # -----------------------------------------------------------------------
    # tst_qml_setup.hpp is listed explicitly for the same reason as
    # solver_test.hpp above - its basename doesn't match tst_qml.cpp, so
    # AUTOMOC's implicit same-name-header heuristic wouldn't find it.
    qt_add_executable(countdown_qml_tests qml/tst_qml.cpp qml/tst_qml_setup.hpp)

    target_link_libraries(countdown_qml_tests
        PRIVATE
            # WHOLE_ARCHIVE on both: countdown_app_core's bundled-dictionary
            # resource and countdown_app_qml's Qt resource-registration static
            # initializers must run even though nothing here calls into either
            # directly - see the qt_add_resources/qt_add_library comments in
            # src/app/CMakeLists.txt for why plain linking silently drops them.
            $<LINK_LIBRARY:WHOLE_ARCHIVE,countdown_app_core>
            $<LINK_LIBRARY:WHOLE_ARCHIVE,countdown_app_qml>
            countdown::warnings
            countdown::sanitizers
            countdown::coverage
            Qt6::QuickTest
            Qt6::Quick
            Qt6::Gui
            Qt6::QuickControls2)

    target_compile_definitions(countdown_qml_tests PRIVATE
        "QUICK_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/qml\"")

    countdown_deploy_sanitizer_runtime(countdown_qml_tests)

    # Same reasoning as countdown_app_tests above, plus --qmldir so
    # windeployqt's static QML scan picks up "import QtTest" from the
    # tst_*.qml files themselves (not just the countdown_app_qml module).
    if(WIN32 AND COUNTDOWN_WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET countdown_qml_tests POST_BUILD
            COMMAND "${COUNTDOWN_WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                --qmldir "${CMAKE_SOURCE_DIR}/src/app/qml"
                --no-translations
                --compiler-runtime
                "$<TARGET_FILE:countdown_qml_tests>"
            COMMENT "Deploying Qt runtime dependencies for countdown_qml_tests")

        # windeployqt only deploys the platform plugin the real windowed app
        # needs (qwindows.dll); this test runs headless in CI via
        # QT_QPA_PLATFORM=offscreen (see set_tests_properties below), which
        # needs qoffscreen.dll present too or Qt aborts before printing
        # anything useful.
        get_filename_component(COUNTDOWN_QT_PLUGINS_DIR "${Qt6_DIR}/../../../plugins" ABSOLUTE)
        if(EXISTS "${COUNTDOWN_QT_PLUGINS_DIR}/platforms/qoffscreen.dll")
            add_custom_command(TARGET countdown_qml_tests POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E make_directory
                    "$<TARGET_FILE_DIR:countdown_qml_tests>/platforms"
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${COUNTDOWN_QT_PLUGINS_DIR}/platforms/qoffscreen.dll"
                    "$<TARGET_FILE_DIR:countdown_qml_tests>/platforms/"
                COMMENT "Deploying the offscreen QPA plugin for countdown_qml_tests")
        else()
            message(WARNING
                "qoffscreen.dll not found under ${COUNTDOWN_QT_PLUGINS_DIR}/platforms; "
                "countdown_qml_tests may fail to start headlessly (QT_QPA_PLATFORM=offscreen).")
        endif()
    endif()

    add_test(NAME countdown_qml_tests COMMAND countdown_qml_tests)

    # QT_QPA_PLATFORM: no display in CI, so run against Qt's software-
    # rendered offscreen platform plugin instead of a real windowing system.
    #
    # ASAN_OPTIONS=detect_leaks=0: LeakSanitizer flags QQmlData/QObject/
    # notifier allocations that Qt's own QML engine and singleton/style
    # caches intentionally keep alive until true process teardown - not our
    # code, and every test function itself passes; this is a well-known
    # false-positive pattern for Qt/QtQuick binaries under ASan. Real
    # memory-safety checks (use-after-free, buffer overflows, ...) stay on.
    set_tests_properties(countdown_qml_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;ASAN_OPTIONS=detect_leaks=0")
endif()
