# ---------------------------------------------------------------------------
# CPack packaging. Included once, from the top-level CMakeLists.txt, after
# every target's install() rules have been declared.
#
# A plain ZIP archive is always offered (needs no external tool). Platform-
# native formats are layered on top when the GUI app is being built and the
# packaging tool for that format is actually available, so `cmake --build .
# --target package` degrades gracefully instead of failing outright.
#
# None of this applies to mobile: real packaging there is androiddeployqt's
# generated APK/AAB and Xcode's archive+export, not CPack. ANDROID also
# implies UNIX, so without this early return a mobile configure would hit
# the UNIX branch below and try (and fail) to build a TGZ/DEB/RPM image.
# ---------------------------------------------------------------------------
if(ANDROID OR IOS)
    message(STATUS
        "Skipping CPack packaging for mobile target (ANDROID/IOS); "
        "use androiddeployqt/Xcode archive+export instead.")
    return()
endif()

set(CPACK_PACKAGE_NAME "CountdownSolver")
set(CPACK_PACKAGE_VENDOR "Iain Chesworth")
set(CPACK_PACKAGE_CONTACT "Iain Chesworth")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CountdownSolver")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_VERBATIM_VARIABLES ON)

set(CPACK_GENERATOR "ZIP")

if(COUNTDOWN_BUILD_APP)
    if(APPLE)
        list(APPEND CPACK_GENERATOR "DragNDrop")
    elseif(WIN32)
        find_program(COUNTDOWN_MAKENSIS_EXECUTABLE makensis)
        if(COUNTDOWN_MAKENSIS_EXECUTABLE)
            list(APPEND CPACK_GENERATOR "NSIS")
            set(CPACK_NSIS_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
            set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        endif()
    elseif(UNIX)
        list(APPEND CPACK_GENERATOR "TGZ")
        find_program(COUNTDOWN_DPKG_DEB_EXECUTABLE dpkg-deb)
        if(COUNTDOWN_DPKG_DEB_EXECUTABLE)
            list(APPEND CPACK_GENERATOR "DEB")
            set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
            set(CPACK_DEBIAN_PACKAGE_SECTION "games")
            set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

            # SHLIBDEPS alone is not enough: dpkg-shlibdeps resolves a shared
            # library to a Depends entry by asking dpkg which *installed apt
            # package* owns that .so file. Qt here comes from a private
            # prebuilt archive (see docs/ci.md / ci/install-qt.*), not an apt
            # package, so dpkg-shlibdeps silently drops every libQt6*.so it
            # finds - confirmed empirically: a real build produced a .deb
            # whose Depends field had every non-Qt shared library correctly
            # listed and *zero* Qt entries. Declare the actual Ubuntu/Debian
            # runtime packages explicitly; SHLIBDEPS still appends whatever
            # it can resolve on its own (glibc, X11, fontconfig, ...)
            # alongside these.
            #
            # Two kinds of package are needed, both empirically confirmed by
            # an `apt install` of a real build: the shared-library packages
            # (apt resolves libqt6core6t64/libqt6gui6t64/libqt6qml6/etc.
            # transitively via just these three), and separately the
            # qml6-module-* packages - Debian/Ubuntu ship each QML module's
            # actual plugin (the qmldir + .so that `import QtQuick.Controls`
            # resolves at runtime) in its own package, apart from the
            # matching shared library.
            #
            # >= 6.8 matters, not just as a floor: this app is built against
            # whatever Qt SDK CI pins (6.8.3 - see docs/ci.md), and its
            # compiled QML metadata hard-requires that same major.minor at
            # runtime (Qt's QML engine refuses an older module version, e.g.
            # "module ... version 6.8 ... QtQuick.Controls.Basic ... is not
            # installed" against a 6.4 runtime - confirmed empirically).
            # Qt 6.8 first appears in Debian 13 (trixie) and Ubuntu 25.04;
            # Ubuntu 24.04 LTS and Debian 12 only have Qt 6.4, so a plain
            # `apt install` there now correctly refuses instead of installing
            # a binary that fails at startup.
            set(CPACK_DEBIAN_PACKAGE_DEPENDS
                "libqt6quick6 (>= 6.8)"
                "libqt6quickcontrols2-6 (>= 6.8)"
                "libqt6svg6 (>= 6.8)"
                "qml6-module-qtquick (>= 6.8)"
                "qml6-module-qtquick-controls (>= 6.8)"
                "qml6-module-qtquick-layouts (>= 6.8)")
        endif()
        find_program(COUNTDOWN_RPMBUILD_EXECUTABLE rpmbuild)
        if(COUNTDOWN_RPMBUILD_EXECUTABLE)
            list(APPEND CPACK_GENERATOR "RPM")
            set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
            set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
            set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)
        endif()
    endif()
endif()

include(CPack)
