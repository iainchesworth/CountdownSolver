#include "platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <utility>

namespace countdown::platform {

PlatformInfo current() {
    // Windows stores per-user application data under %APPDATA%.
    std::filesystem::path config_dir;
    if (const char* appdata = std::getenv("APPDATA")) {
        config_dir = std::filesystem::path(appdata) / "CountdownSolver";
    }
    return PlatformInfo{"Windows", std::move(config_dir)};
}

void ensure_console_output() {
    // A GUI-subsystem binary has no console; if launched from one, reuse it.
    if (AttachConsole(ATTACH_PARENT_PROCESS) != 0) {
        std::FILE* stream = nullptr;
        (void)freopen_s(&stream, "CONOUT$", "w", stdout);
        (void)freopen_s(&stream, "CONOUT$", "w", stderr);
    }
}

}  // namespace countdown::platform
