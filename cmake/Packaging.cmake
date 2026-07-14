# ---------------------------------------------------------------------------
# CPack packaging. Included once, from the top-level CMakeLists.txt, after
# every target's install() rules have been declared.
#
# A plain ZIP archive is always offered (needs no external tool). Platform-
# native formats are layered on top when the GUI app is being built and the
# packaging tool for that format is actually available, so `cmake --build .
# --target package` degrades gracefully instead of failing outright.
# ---------------------------------------------------------------------------
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
            # listed and *zero* Qt entries. Declare the actual Ubuntu runtime
            # packages explicitly; SHLIBDEPS still appends whatever it can
            # resolve on its own (glibc, X11, fontconfig, ...) alongside
            # these. Depending on just these three is enough: apt resolves
            # libqt6core6t64/libqt6gui6t64/libqt6qml6/etc. transitively via
            # their own Depends.
            set(CPACK_DEBIAN_PACKAGE_DEPENDS
                "libqt6quick6"
                "libqt6quickcontrols2-6"
                "libqt6svg6")
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
