#pragma once

#include <filesystem>
#include <string>

namespace countdown::platform {

// Host-specific facts the application needs. There is exactly one definition of
// `current()` per platform, living in platform/<os>/. CMake compiles only the
// file matching the target OS, so the code contains no preprocessor branching.
struct PlatformInfo {
    std::string name;                  // Human-readable OS name.
    std::filesystem::path config_dir;  // Per-user configuration directory.
};

[[nodiscard]] PlatformInfo current();

// Ensures standard output is connected to a terminal when the process was
// launched from one. On a Windows GUI-subsystem binary this attaches to the
// parent console so `--version`/`--help` text is visible; elsewhere it is a
// no-op. Isolating the Win32 call here keeps the rest of the app #ifdef-free.
void ensure_console_output();

}  // namespace countdown::platform
