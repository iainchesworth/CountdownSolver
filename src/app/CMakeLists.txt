# ---------------------------------------------------------------------------
# countdown_app - the Qt6 GUI. All logic lives in countdown::solver; this
# target is only presentation plus platform glue.
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

# Select the single platform implementation for the target OS. No #ifdefs: CMake
# decides which translation unit is compiled.
if(WIN32)
    set(COUNTDOWN_PLATFORM_SOURCE platform/windows/platform_windows.cpp)
elseif(APPLE)
    set(COUNTDOWN_PLATFORM_SOURCE platform/macos/platform_macos.cpp)
elseif(UNIX)
    set(COUNTDOWN_PLATFORM_SOURCE platform/linux/platform_linux.cpp)
else()
    message(FATAL_ERROR "Unsupported target platform")
endif()

qt_add_executable(countdown_app
    main.cpp
    main_window.cpp
    main_window.hpp
    platform/platform.hpp
    ${COUNTDOWN_PLATFORM_SOURCE})

target_include_directories(countdown_app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(countdown_app
    PRIVATE
        countdown::solver
        countdown::warnings
        countdown::sanitizers
        Qt6::Widgets)

# std::getenv is flagged as "unsafe" by the MSVC CRT; the usage here is correct
# and portable, so silence that specific deprecation for this target only.
target_compile_definitions(countdown_app PRIVATE
    "$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>")

set_target_properties(countdown_app PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON)
