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
    unit/version_test.cpp
    unit/alphabet_test.cpp
    unit/dictionary_i18n_test.cpp
    unit/utf8_test_support.hpp)

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
# Benchmarks: Catch2's own BENCHMARK() macros, timing the library's hot
# paths against the real bundled dictionary (122k+ words) - the same data
# every Solver loads, not the small tests/data/ fixture. Deliberately NOT
# registered with catch_discover_tests/add_test: statistical timing isn't a
# pass/fail CTest gate, and running it under Debug+ASan (as every CI preset
# does for the suites above) would make its numbers meaningless - build and
# run `countdown_benchmark_tests` directly, ideally from a Release configure,
# when checking for a regression like the one that made a single
# countdownsolver_tests slot rebuild this same dictionary 15 times over (see
# app_tests_main.cpp's shared Solver instance, in tests/app/).
# ---------------------------------------------------------------------------
add_executable(countdown_benchmark_tests
    benchmark/dictionary_benchmark.cpp)

target_compile_definitions(countdown_benchmark_tests PRIVATE
    "COUNTDOWN_DICTIONARY_PATH=\"${CMAKE_SOURCE_DIR}/src/app/resources/dictionary/words_en.txt\"")

target_link_libraries(countdown_benchmark_tests
    PRIVATE
        countdown::solver
        countdown::warnings
        Catch2::Catch2WithMain)

# ---------------------------------------------------------------------------
# App tests: exercise the Qt-facing Solver facade (countdownsolver_core) with
# Qt Test, without linking Quick/Gui/QuickControls2. Only built alongside the
# app itself, since countdownsolver_core only exists when COUNTDOWN_BUILD_APP
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

    # One QtTest class per Solver concern (numbers/letters/conundrum/rack/
    # version/dictionary/logging) rather than one lumped class with every
    # slot in it, so a failure narrows straight to the concern that broke
    # instead of forcing a scroll through one large Qt Test report. All
    # still link into a single countdownsolver_tests binary/CTest entry -
    # app_tests_main.cpp's hand-written main() runs each class in turn (see
    # its own comment for why that replaces QTEST_APPLESS_MAIN). Every *.hpp
    # is listed explicitly (not just #included) so AUTOMOC reliably scans it
    # for Q_OBJECT regardless of basename-matching quirks.
    qt_add_executable(countdownsolver_tests
        app/app_tests_main.cpp
        app/solver_numbers_test.cpp
        app/solver_numbers_test.hpp
        app/solver_letters_test.cpp
        app/solver_letters_test.hpp
        app/solver_conundrum_test.cpp
        app/solver_conundrum_test.hpp
        app/solver_rack_test.cpp
        app/solver_rack_test.hpp
        app/solver_version_test.cpp
        app/solver_version_test.hpp
        app/solver_dictionary_test.cpp
        app/solver_dictionary_test.hpp
        app/solver_logging_test.cpp
        app/solver_logging_test.hpp
        app/solver_test_utils.hpp)

    target_link_libraries(countdownsolver_tests
        PRIVATE
            # WHOLE_ARCHIVE: countdownsolver_core's bundled-dictionary resource
            # object file has no direct symbol references, so a plain link
            # would let the linker silently drop it - see the qt_add_resources
            # comment in src/app/CMakeLists.txt.
            $<LINK_LIBRARY:WHOLE_ARCHIVE,countdownsolver_core>
            countdown::warnings
            countdown::sanitizers
            countdown::coverage
            Qt6::Test)

    countdown_deploy_sanitizer_runtime(countdownsolver_tests)

    # windeployqt only runs against countdownsolver (see src/app/CMakeLists.txt)
    # and only copies the Qt DLLs *that target* depends on, so Qt6Test.dll
    # never lands in the shared bin/ output directory otherwise: the test exe
    # fails to start (STATUS_DLL_NOT_FOUND) even though it links fine.
    if(WIN32 AND COUNTDOWN_WINDEPLOYQT_EXECUTABLE)
        # countdownsolver, countdownsolver_tests and countdown_qml_tests each run a
        # windeployqt POST_BUILD step that writes into the same shared
        # CMAKE_RUNTIME_OUTPUT_DIRECTORY (bin/qml, bin/platforms, ...). Ninja
        # otherwise links + deploys all three concurrently, and two windeployqt
        # processes race to create the same destination file - observed in CI as
        # "Cannot create ... QtQuick/Controls/Fusion/RadioButton.qml ... file
        # already exists" (PR #12). add_dependencies() forces countdownsolver to
        # fully build (link *and* deploy) before countdownsolver_tests starts;
        # countdown_qml_tests below chains behind countdownsolver_tests the same
        # way, giving a total order across all three windeployqt invocations.
        add_dependencies(countdownsolver_tests countdownsolver)

        add_custom_command(TARGET countdownsolver_tests POST_BUILD
            COMMAND "${COUNTDOWN_WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --compiler-runtime
                "$<TARGET_FILE:countdownsolver_tests>"
            COMMENT "Deploying Qt runtime dependencies for countdownsolver_tests")

        # Every windeployqt invocation writes into the same shared
        # CMAKE_RUNTIME_OUTPUT_DIRECTORY (bin/), but each is a separate
        # POST_BUILD command tied to its own target, so ninja has no
        # visibility into that shared file-level side effect and may run two
        # invocations concurrently - one then fails with "Cannot create a
        # file when that file already exists" (same class of race already
        # hit once for the ASan runtime DLL copy; see
        # countdown_deploy_sanitizer_runtime in Sanitizers.cmake). Forcing an
        # explicit target dependency on the previous windeployqt-using
        # target in the chain (countdownsolver here) makes ninja serialize
        # them instead.
        add_dependencies(countdownsolver_tests countdownsolver)
    endif()

    add_test(NAME countdownsolver_tests COMMAND countdownsolver_tests)

    # -----------------------------------------------------------------------
    # QML tests: Qt Quick Test drives the real "Countdown" QML module (pages +
    # solver wiring) headlessly. countdownsolver_qml is the same compiled module
    # countdownsolver itself links, so this exercises the exact QML shipped -
    # no separate copy of the QML_FILES list.
    # -----------------------------------------------------------------------
    # tst_qml_setup.hpp is listed explicitly for the same reason as
    # solver_test.hpp above - its basename doesn't match tst_qml.cpp, so
    # AUTOMOC's implicit same-name-header heuristic wouldn't find it.
    # language_manager.cpp/.hpp aren't part of countdownsolver_core (that target
    # deliberately stays free of Quick/Gui - see its comment in
    # src/app/CMakeLists.txt), so they're compiled in directly here, the same
    # way countdownsolver itself does it - countdown_qml_tests already links
    # Quick/Gui/QuickControls2 to run Quick Test in the first place, and
    # tst_qml_setup.hpp registers a real LanguageManager as the
    # `languageManager` context property so Theme.qml/SettingsPage.qml's
    # bindings resolve exactly as they do in the real app.
    qt_add_executable(countdown_qml_tests
        qml/tst_qml.cpp
        qml/tst_qml_setup.hpp
        "${CMAKE_SOURCE_DIR}/src/app/language_manager.cpp"
        "${CMAKE_SOURCE_DIR}/src/app/language_manager.hpp")

    target_link_libraries(countdown_qml_tests
        PRIVATE
            # WHOLE_ARCHIVE on both: countdownsolver_core's bundled-dictionary
            # resource and countdownsolver_qml's Qt resource-registration static
            # initializers must run even though nothing here calls into either
            # directly - see the qt_add_resources/qt_add_library comments in
            # src/app/CMakeLists.txt for why plain linking silently drops them.
            $<LINK_LIBRARY:WHOLE_ARCHIVE,countdownsolver_core>
            $<LINK_LIBRARY:WHOLE_ARCHIVE,countdownsolver_qml>
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

    # Same reasoning as countdownsolver_tests above, plus --qmldir so
    # windeployqt's static QML scan picks up "import QtTest" from the
    # tst_*.qml files themselves (not just the countdownsolver_qml module).
    if(WIN32 AND COUNTDOWN_WINDEPLOYQT_EXECUTABLE)
        # Chains behind countdownsolver_tests's windeployqt step - see the
        # add_dependencies() comment above countdownsolver_tests's windeployqt
        # block for why.
        add_dependencies(countdown_qml_tests countdownsolver_tests)

        add_custom_command(TARGET countdown_qml_tests POST_BUILD
            COMMAND "${COUNTDOWN_WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                --qmldir "${CMAKE_SOURCE_DIR}/src/app/qml"
                --no-translations
                --compiler-runtime
                "$<TARGET_FILE:countdown_qml_tests>"
            COMMENT "Deploying Qt runtime dependencies for countdown_qml_tests")

        # See the countdownsolver_tests windeployqt comment above: chain this
        # invocation after the previous one so ninja never runs two
        # windeployqt copies into bin/ concurrently.
        add_dependencies(countdown_qml_tests countdownsolver_tests)

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

    # One CTest entry per page rather than one countdown_qml_tests entry
    # covering all four - a failing page names itself in CI instead of
    # forcing a scroll through combined Quick Test output to find which
    # TestCase broke. All four share the single countdown_qml_tests binary
    # built above (so only one windeployqt/offscreen-plugin deployment is
    # needed); Qt Quick Test's "-input dir/file" option accepts a single QML
    # file as well as a directory, so each entry just restricts that shared
    # binary to its own tst_*.qml file.
    set(COUNTDOWN_QML_TEST_PAGES ConundrumPage LettersPage NumbersPage SettingsPage Theme LanguageManager LanguageSwitching)

    foreach(page IN LISTS COUNTDOWN_QML_TEST_PAGES)
        add_test(NAME countdown_qml_tests_${page}
            COMMAND countdown_qml_tests -input "${CMAKE_CURRENT_SOURCE_DIR}/qml/tst_${page}.qml")

        # QT_QPA_PLATFORM: no display in CI, so run against Qt's software-
        # rendered offscreen platform plugin instead of a real windowing
        # system.
        #
        # ASAN_OPTIONS=detect_leaks=0: LeakSanitizer flags QQmlData/QObject/
        # notifier allocations that Qt's own QML engine and singleton/style
        # caches intentionally keep alive until true process teardown - not
        # our code, and every test function itself passes; this is a
        # well-known false-positive pattern for Qt/QtQuick binaries under
        # ASan. Real memory-safety checks (use-after-free, buffer overflows,
        # ...) stay on.
        set_tests_properties(countdown_qml_tests_${page} PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;ASAN_OPTIONS=detect_leaks=0")
    endforeach()
endif()
