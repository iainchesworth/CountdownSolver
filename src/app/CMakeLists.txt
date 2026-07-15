# ---------------------------------------------------------------------------
# countdownsolver - the Qt6 Quick (QML) GUI. All game logic lives in
# countdown::solver; this target is presentation + platform glue only.
# ---------------------------------------------------------------------------
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Concurrent Quick Gui QuickControls2 LinguistTools)

# CI-confirmed Qt 6.8.3-for-iOS-kit packaging gap: ~30 plugin targets (image
# format plugins, platform integration plugins, qmltooling debug/profiler
# plugins, and the Apple-specific permission/TLS/reachability plugins) don't
# export QT_DEFAULT_PLUGIN at all. Qt6/QtPublicPluginHelpers.cmake's plugin
# auto-linkage genex reads that property directly, unwrapped, as one operand
# of a $<AND:...>; an unset property is an empty string, and CMake's Generate
# phase hard-errors ("Parameters to $<AND> must resolve to either '0' or
# '1'") rather than treating empty as falsy - confirmed identical on both
# CMake 4.3.3 and 4.0.3, ruling out a CMake-version-specific cause. No
# matching upstream Qt bug report found for this exact error. The desktop Qt
# 6.8.3 kit exports this property correctly on every one of these plugins
# that exists there too (verified directly against that kit's own
# Targets.cmake files); restore the same values it uses. The genex is
# evaluated lazily at Generate time, so setting the property here - any
# point after find_package(Qt6) creates these IMPORTED targets - is visible
# by the time it's read, regardless of directory/scope. iOS-only: none of
# these targets exist on other platforms, so the if(TARGET ...) guards make
# this an inert no-op everywhere else.
if(IOS)
    # Matches the desktop Qt 6.8.3 kit's own QT_DEFAULT_PLUGIN value for each
    # (image format plugins, qmltooling debug/profiler plugins), plus a
    # reasoned default for the six Apple-only plugins that have no desktop
    # equivalent to compare against: "1" for the platform-integration plugin
    # (QIOSIntegrationPlugin, the iOS analogue of QWindowsIntegrationPlugin/
    # QCocoaIntegrationPlugin - required for the app to run at all), the TLS
    # backend (QSecureTransportBackendPlugin, the Apple analogue of
    # QSchannelBackendPlugin/QTlsBackendOpenSSLPlugin - needed for HTTPS to
    # work out of the box), and network reachability monitoring
    # (QSCNetworkReachabilityNetworkInformationPlugin, base infrastructure,
    # not an opt-in feature).
    foreach(_countdown_ios_plugin IN ITEMS
            QDebugMessageServiceFactoryPlugin
            QGifPlugin
            QICOPlugin
            QIOSIntegrationPlugin
            QJpegPlugin
            QLocalClientConnectionFactoryPlugin
            QQmlDebugServerFactoryPlugin
            QQmlDebuggerServiceFactoryPlugin
            QQmlInspectorServiceFactoryPlugin
            QQmlNativeDebugConnectorFactoryPlugin
            QQmlNativeDebugServiceFactoryPlugin
            QQmlPreviewServiceFactoryPlugin
            QQmlProfilerServiceFactoryPlugin
            QQuickProfilerAdapterFactoryPlugin
            QSCNetworkReachabilityNetworkInformationPlugin
            QSecureTransportBackendPlugin
            QSvgIconPlugin
            QSvgPlugin
            QTcpServerConnectionFactoryPlugin)
        if(TARGET Qt6::${_countdown_ios_plugin})
            set_target_properties(Qt6::${_countdown_ios_plugin} PROPERTIES QT_DEFAULT_PLUGIN "1")
        endif()
    endforeach()

    # "0" (opt-in only, not auto-linked): matches the desktop kit's own value
    # for the platform-integration alternates and TUIO touch plugin; for the
    # six Darwin permission plugins and the optional photo-library picker,
    # this app requests none of the matching Info.plist usage-description
    # keys (NSCameraUsageDescription etc.), so auto-linking them would risk a
    # runtime crash the first time Qt's permission-check code path is hit
    # without one present - "0" here isn't just a safe guess, it matches
    # this app's actual Info.plist.in content.
    foreach(_countdown_ios_plugin IN ITEMS
            QDarwinBluetoothPermissionPlugin
            QDarwinCalendarPermissionPlugin
            QDarwinCameraPermissionPlugin
            QDarwinContactsPermissionPlugin
            QDarwinLocationPermissionPlugin
            QDarwinMicrophonePermissionPlugin
            QIosOptionalPlugin_NSPhotoLibraryPlugin
            QMinimalIntegrationPlugin
            QOffscreenIntegrationPlugin
            QTlsBackendCertOnlyPlugin
            QTuioTouchPlugin)
        if(TARGET Qt6::${_countdown_ios_plugin})
            set_target_properties(Qt6::${_countdown_ios_plugin} PROPERTIES QT_DEFAULT_PLUGIN "0")
        endif()
    endforeach()
endif()

qt_standard_project_setup(REQUIRES 6.5)

# QML files live under qml/components/ and qml/pages/ as well as qml/ itself;
# NEW makes qt_add_qml_module generate a qmldir for each of those extra
# directories so types resolve without hand-written qmldir files.
qt_policy(SET QTP0004 NEW)

# Select the single platform implementation for the target OS. No #ifdefs: CMake
# decides which translation unit is compiled. ANDROID/IOS must be checked
# before WIN32/APPLE/UNIX: CMake sets ANDROID alongside UNIX, and IOS alongside
# APPLE, when cross-compiling for those targets.
if(ANDROID)
    set(COUNTDOWN_PLATFORM_SOURCE platform/android/platform_android.cpp)
elseif(IOS)
    set(COUNTDOWN_PLATFORM_SOURCE platform/ios/platform_ios.cpp)
elseif(WIN32)
    set(COUNTDOWN_PLATFORM_SOURCE platform/windows/platform_windows.cpp)
elseif(APPLE)
    set(COUNTDOWN_PLATFORM_SOURCE platform/macos/platform_macos.cpp)
elseif(UNIX)
    set(COUNTDOWN_PLATFORM_SOURCE platform/linux/platform_linux.cpp)
else()
    message(FATAL_ERROR "Unsupported target platform")
endif()

# ---------------------------------------------------------------------------
# countdownsolver_core - the Solver QObject facade + platform glue, split out
# as a static library so it can be linked into a Qt Test binary (tests/app)
# without pulling in Quick/Gui/QuickControls2. countdownsolver below is then
# just the QML-loading executable shell over this core.
# ---------------------------------------------------------------------------
qt_add_library(countdownsolver_core STATIC
    solver.cpp
    solver.hpp
    platform/platform.hpp
    logging/logging.hpp
    logging/logging.cpp
    ${COUNTDOWN_PLATFORM_SOURCE})

target_include_directories(countdownsolver_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(countdownsolver_core
    PUBLIC
        countdown::solver
        Qt6::Core
        Qt6::Concurrent
    PRIVATE
        countdown::warnings
        countdown::sanitizers
        countdown::coverage)

# std::getenv is flagged "unsafe" by the MSVC CRT; the usage is correct and
# portable, so silence that specific deprecation for this target only. This
# also fires under clang-cl (CXX_COMPILER_ID "Clang"), which uses the same
# MSVC CRT headers/annotations on Windows.
target_compile_definitions(countdownsolver_core PRIVATE
    "$<$<OR:$<CXX_COMPILER_ID:MSVC>,$<AND:$<PLATFORM_ID:Windows>,$<CXX_COMPILER_ID:Clang>>>:_CRT_SECURE_NO_WARNINGS>")

# The version header is generated by the library's build-time stamp target.
add_dependencies(countdownsolver_core countdown_version)

# Bundles the default word list into countdownsolver_core (not just the
# countdownsolver executable) as a Qt resource, so anything that constructs a
# Solver - the real app, but also tests/app and tests/qml's Qt Test binaries,
# which link countdownsolver_core directly and never link countdownsolver itself -
# gets a complete dictionary. Consumers must link this target with
# $<LINK_LIBRARY:WHOLE_ARCHIVE,countdownsolver_core> (see below): like
# countdownsolver_qml, nothing calls into the generated resource-registration
# object file directly, so a plain link would let the archive-member-elision
# linker default drop it and QFile(":/dictionary/words_en.txt") would silently
# open nothing.
qt_add_resources(countdownsolver_core "dictionary"
    PREFIX "/dictionary"
    BASE "resources/dictionary"
    FILES
        resources/dictionary/words_en.txt
        resources/dictionary/words_fr.txt
        resources/dictionary/words_de.txt
        resources/dictionary/words_es.txt
        resources/dictionary/words_ar.txt
        resources/dictionary/words_he.txt
        resources/dictionary/words_yi.txt)

# ---------------------------------------------------------------------------
# countdownsolver_qml - the "Countdown" QML module (Main shell, Theme/AppState
# singletons, components, pages), as its own static backing library rather
# than embedded directly in countdownsolver. This lets tests/qml link the exact
# same compiled module into a separate Qt Quick Test executable without
# duplicating the QML_FILES list against out-of-tree paths.
#
# Two things this needs that a plain STATIC + target_link_libraries doesn't
# give for free:
#   - A SHARED backing library ends up with no direct symbol references from
#     countdownsolver (all interaction is runtime QML resource lookup, not C++
#     calls), so the default GCC/Clang `--as-needed` linker behavior drops it
#     from DT_NEEDED entirely - the plugin .so never loads, so its Qt
#     resource-registration static initializers never run, and `import
#     Countdown` fails at runtime with "qmldir file not found".
#   - qt_add_qml_module's "no separate plugin" resource-embedding only
#     applies when the backing target IS the final executable; for a
#     STATIC library consumed by a *dynamically*-linked prebuilt Qt (as used
#     here), qt6_import_qml_plugins() - the usual fix for statically-linked
#     QML modules - is designed for fully static Qt builds and doesn't import
#     it either.
# STATIC + forcing the archive to link in full (see the WHOLE_ARCHIVE wraps
# below) sidesteps both: the module's resource-registration object files are
# always retained regardless of symbol usage, with no runtime plugin-file
# discovery/dlopen path to get wrong.
# ---------------------------------------------------------------------------
qt_add_library(countdownsolver_qml STATIC)

# Theme and AppState are QML singletons. This must run before
# qt_add_qml_module below: that call reads QT_QML_SINGLETON_TYPE off each
# QML_FILES entry to decide whether to emit a `singleton` qmldir line, so
# setting it afterwards silently registers both as plain (non-singleton)
# types and every Theme.*/AppState.* binding in the app resolves to
# undefined at runtime.
set_source_files_properties(qml/Theme.qml qml/AppState.qml qml/FormFactor.qml qml/Metrics.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(countdownsolver_qml
    URI Countdown
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/AppState.qml
        qml/FormFactor.qml
        qml/Metrics.qml
        qml/components/Card.qml
        qml/components/SectionLabel.qml
        qml/components/Tile.qml
        qml/components/PadButton.qml
        qml/components/FlatButton.qml
        qml/components/NavItem.qml
        qml/components/SegControl.qml
        qml/components/SegChip.qml
        qml/components/AccentChip.qml
        qml/components/OnScreenKeyboard.qml
        qml/components/LicensesDialog.qml
        qml/components/BottomTabButton.qml
        qml/components/BottomTabBar.qml
        qml/components/ViewToggle.qml
        qml/components/PageHeader.qml
        qml/pages/NumbersPage.qml
        qml/pages/LettersPage.qml
        qml/pages/ConundrumPage.qml
        qml/pages/SettingsPage.qml)

# The window icon (taskbar/titlebar chrome at runtime, all platforms) is
# bundled as a Qt resource so main.cpp can load it as ":/icon/app_256.png"
# and hand it to QQuickWindow::setIcon() regardless of where the app is
# installed (QQuickWindow has no QML-facing `icon` property - see the
# comment in main.cpp). This resource is separate from the Windows .rc/.ico
# below, which only covers the exe file's own icon (Explorer, Alt+Tab before
# any window exists).
qt_add_resources(countdownsolver_qml "icon"
    PREFIX "/icon"
    BASE resources/icon
    FILES resources/icon/app_256.png)

# Windows exe icon (window/taskbar chrome + Explorer). qt_add_executable
# happily compiles a .rc source alongside main.cpp; the .ico it references
# is looked up relative to the .rc file's own directory by rc.exe, not the
# build dir, so no extra copy step is needed.
if(WIN32)
    set(COUNTDOWN_SOLVER_ICON_SOURCE resources/icon/app.rc)
endif()

qt_add_executable(countdownsolver
    main.cpp
    language_manager.cpp
    language_manager.hpp
    ${COUNTDOWN_SOLVER_ICON_SOURCE})

# ---------------------------------------------------------------------------
# Translations: French, German, Spanish, Arabic, Hebrew and Yiddish UI text.
# qt_add_translations() drives lupdate (scan for tr()/qsTr() calls, update the
# .ts files) and lrelease (compile them to .qm) at build time, and registers
# the .qm files as Qt resources so they're embedded in the binary. SOURCES is
# listed explicitly - the QML files already belong to countdownsolver_qml's
# sources so lupdate would find them either way, but solver.cpp's tr() calls
# live in the separate countdownsolver_core target and wouldn't be scanned
# without naming it here.
#
# Placed after qt_add_executable() above, WITH IMMEDIATE_CALL - both matter.
# Without IMMEDIATE_CALL, qt_add_translations() defers itself to run at the
# end of PROJECT_SOURCE_DIR's scope (the repo root), rather than running here
# in src/app's scope. That, in turn, made Qt6CoreMacros.cmake's
# _qt_internal_process_resource() (called internally to embed the .qm files
# as a resource on countdownsolver_qml) see CMAKE_CURRENT_SOURCE_DIR as the
# repo root while countdownsolver_qml's own SOURCE_DIR is src/app - a
# mismatch that macro treats as "the resource is being attached from a
# different directory than the target lives in", so it defensively wraps the
# embedding in a brand new target (named countdownsolver_qml_<resourcename>,
# i.e. countdownsolver_qml_countdownsolver_qml_translations) rather than
# attaching the generated file to countdownsolver_qml directly. That wrapper
# target and CountdownSolver_lrelease (the plain custom target
# qt_add_translations() also creates, to actually run lrelease) then both
# independently reference the same countdown_<lang>.qm output with no
# dependency edge between them - which Ninja tolerates (a file may be a
# dependency of more than one target) but Xcode's "new build system" rejects
# outright ("is attached to multiple targets ... not allowed by the Xcode
# 'new build system'"), confirmed against a real Qt 6.8.3/Xcode 16.4 CI run.
# IMMEDIATE_CALL runs this synchronously right here in src/app's own scope,
# matching countdownsolver_qml's SOURCE_DIR exactly, so that mismatch (and
# the wrapper target it produces) never arises in the first place - fixing
# the root cause rather than reactively wiring a dependency onto a target
# that isn't reliably visible to CMake script code by the time it exists
# (the failure mode of every previous attempt here, per git history).
# SOURCE_TARGETS isn't passed explicitly since IMMEDIATE_CALL's own
# auto-collection (qt6_collect_translation_source_targets) already walks
# every BUILDSYSTEM_TARGETS entry under CMAKE_CURRENT_SOURCE_DIR at the
# point it runs - src/app - which is why this must come after
# qt_add_executable() above, so countdownsolver itself (and its main.cpp/
# language_manager.cpp sources) is included in that scan too, not just
# countdownsolver_core and countdownsolver_qml.
#
# Behaviorally a no-op for every other platform here (Windows/Linux/macOS/
# Android): none of them care whether the .qm embedding goes through the
# extra wrapper target or attaches directly, so this isn't scoped to
# if(IOS) - it's just a strictly better outcome everywhere.
# ---------------------------------------------------------------------------
qt_add_translations(countdownsolver_qml
    IMMEDIATE_CALL
    TS_FILES
        translations/countdown_fr.ts
        translations/countdown_de.ts
        translations/countdown_es.ts
        translations/countdown_ar.ts
        translations/countdown_he.ts
        translations/countdown_yi.ts
    SOURCES
        qml/Main.qml
        qml/pages/NumbersPage.qml
        qml/pages/LettersPage.qml
        qml/pages/ConundrumPage.qml
        qml/pages/SettingsPage.qml
        solver.cpp)

# QT_ANDROID_PACKAGE_SOURCE_DIR points Qt's CMake integration at the directory
# holding AndroidManifest.xml (and androiddeployqt's other package inputs);
# MACOSX_BUNDLE_INFO_PLIST is CMake's own bundle-Info.plist mechanism, which
# also drives iOS bundles under the Xcode generator (MACOSX_BUNDLE ON below
# already triggers bundle output there). TARGETED_DEVICE_FAMILY "2" restricts
# the iOS build to iPad only, matching the tablet-only scope of this app.
if(ANDROID)
    set_target_properties(countdownsolver PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform/android")
endif()

if(IOS)
    set_target_properties(countdownsolver PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platform/ios/Info.plist.in"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "2")
endif()

# IBM Plex Sans/Mono (Theme.qml's Theme.sans/Theme.mono) plus Noto Sans
# Arabic/Hebrew (all SIL Open Font License - see the accompanying *-OFL.txt
# files), bundled so text renders with the intended typeface and glyph
# coverage regardless of what's installed on the host, rather than silently
# depending on IBM Plex happening to already be present system-wide (it
# isn't, on Android or most other machines - confirmed the missing bundle
# was the root cause of washed-out/inconsistent-weight text and a missing
# backspace glyph on Android, none of which reproduced on a dev machine that
# happened to have IBM Plex installed already). IBM Plex Sans has no Arabic
# or Hebrew coverage, hence the separate Noto fallbacks. Both Sans and Mono
# are bundled as four static weights (Regular/Medium/SemiBold/Bold - the
# only ones Theme.sans/Theme.mono are actually used at), not variable fonts:
# IBM Plex Mono ships no variable build at all, and IBM Plex Sans's variable
# build registers under the family name "IBM Plex Sans Var" rather than
# "IBM Plex Sans" - confirmed by inspecting its name table - which wouldn't
# match Theme.qml's Theme.sans string at all. Only countdownsolver needs
# these - no test target renders text - so unlike the dictionary resource
# above, this lives directly on the final executable with no WHOLE_ARCHIVE
# dance required.
qt_add_resources(countdownsolver "fonts"
    PREFIX "/fonts"
    BASE "resources/fonts"
    FILES
        resources/fonts/IBMPlexSans-Regular.ttf
        resources/fonts/IBMPlexSans-Medium.ttf
        resources/fonts/IBMPlexSans-SemiBold.ttf
        resources/fonts/IBMPlexSans-Bold.ttf
        resources/fonts/IBMPlexMono-Regular.ttf
        resources/fonts/IBMPlexMono-Medium.ttf
        resources/fonts/IBMPlexMono-SemiBold.ttf
        resources/fonts/IBMPlexMono-Bold.ttf
        resources/fonts/NotoSansArabic.ttf
        resources/fonts/NotoSansHebrew.ttf)

target_link_libraries(countdownsolver
    PRIVATE
        $<LINK_LIBRARY:WHOLE_ARCHIVE,countdownsolver_core>
        $<LINK_LIBRARY:WHOLE_ARCHIVE,countdownsolver_qml>
        countdown::warnings
        countdown::sanitizers
        countdown::coverage
        Qt6::Quick
        Qt6::Gui
        Qt6::QuickControls2)

# The version header is generated by the library's build-time stamp target.
add_dependencies(countdownsolver countdown_version)

countdown_deploy_sanitizer_runtime(countdownsolver)

set_target_properties(countdownsolver PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON)

# ---------------------------------------------------------------------------
# Deploy Qt's runtime DLLs and QML plugins (Qt6Core/Gui/Qml/Quick, the
# platform plugin, QtQuick.Controls/Layouts, ...) next to the built exe so it
# runs directly from the build tree, matching how it would ship. Without
# this, countdownsolver.exe fails to start: the loader can't find the Qt DLLs
# and QML can't resolve QtQuick.Controls/Layouts imports.
# ---------------------------------------------------------------------------
if(WIN32)
    get_filename_component(COUNTDOWN_QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)
    find_program(COUNTDOWN_WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt6 windeployqt
        HINTS "${COUNTDOWN_QT_BIN_DIR}")

    if(COUNTDOWN_WINDEPLOYQT_EXECUTABLE)
        # countdownsolver_tests and countdown_qml_tests (tests/CMakeLists.txt) run
        # their own windeployqt POST_BUILD step against this same
        # CMAKE_RUNTIME_OUTPUT_DIRECTORY and add_dependencies() on this target to
        # serialize behind it - see the comment there for why (concurrent
        # windeployqt processes race on shared destination files).
        add_custom_command(TARGET countdownsolver POST_BUILD
            COMMAND "${COUNTDOWN_WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                --compiler-runtime
                "$<TARGET_FILE:countdownsolver>"
            COMMENT "Deploying Qt runtime dependencies for countdownsolver")
    else()
        message(WARNING
            "windeployqt not found next to Qt6 (looked in ${COUNTDOWN_QT_BIN_DIR}); "
            "countdownsolver.exe will not run standalone from the build tree.")
    endif()
endif()

# ---------------------------------------------------------------------------
# Install + Qt runtime deployment for packaging. qt_generate_deploy_app_script()
# is Qt's CMake-native replacement for windeployqt/macdeployqt at install
# time: it inspects the built target and emits an install(SCRIPT) that copies
# the Qt libraries/plugins the app actually uses next to it (or into the .app
# bundle on macOS). This is separate from the POST_BUILD windeployqt step
# above, which targets the build tree, not an install/package tree.
#
# Linux gets none of this: it has no supported deploy step, and Linux
# packages rely on a system Qt6 install instead (see the DEB/RPM Depends in
# cmake/Packaging.cmake). NO_UNSUPPORTED_PLATFORM_ERROR would turn the
# unsupported-platform library copy into a no-op rather than a configure
# error, but it does NOT stop qt_generate_deploy_app_script() from still
# writing a "Prefix = .." qt.conf next to the installed binary - confirmed
# empirically: that qt.conf, meant for a self-contained relocatable bundle
# (Windows/macOS), instead misdirects the system Qt6 library's QML import
# path lookup on Linux (e.g. to "/usr/qml" instead of the real multiarch
# "/usr/lib/x86_64-linux-gnu/qt6/qml"), breaking QtQuick.Controls at
# startup even when a compatible Qt6 is installed. Skip the whole call on
# Linux, where it has nothing useful left to do anyway.
#
# Mobile (ANDROID/IOS) skips this entire block, same rationale as the CPack
# early-return in cmake/Packaging.cmake: real mobile packaging is
# androiddeployqt's generated APK/AAB and Xcode's archive+export, neither of
# which goes through `cmake --install`. Worse, on Android qt_add_executable()
# produces a MODULE (shared library) target rather than a true executable,
# so install(TARGETS ... RUNTIME DESTINATION ...) with no LIBRARY DESTINATION
# is a hard configure-time error there.
# ---------------------------------------------------------------------------
if(NOT (ANDROID OR IOS))
    include(GNUInstallDirs)

    install(TARGETS countdownsolver
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        BUNDLE DESTINATION ".")

    if(WIN32 OR APPLE)
        qt_generate_deploy_app_script(
            TARGET countdownsolver
            OUTPUT_SCRIPT COUNTDOWN_SOLVER_DEPLOY_SCRIPT
            NO_UNSUPPORTED_PLATFORM_ERROR)
        install(SCRIPT "${COUNTDOWN_SOLVER_DEPLOY_SCRIPT}")
    endif()
endif()
