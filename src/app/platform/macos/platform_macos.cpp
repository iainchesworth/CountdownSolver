#include "platform/platform.hpp"

#include <cstdlib>
#include <filesystem>
#include <utility>

namespace countdown::platform {

PlatformInfo current() {
    // macOS convention: ~/Library/Application Support/<app>.
    std::filesystem::path config_dir;
    if (const char* home = std::getenv("HOME")) {
        config_dir = std::filesystem::path(home) / "Library" / "Application Support" / "CountdownSolver";
    }
    return PlatformInfo{"macOS", std::move(config_dir)};
}

void ensure_console_output() {
    // On macOS the process already inherits the launching terminal's stdout.
}

}  // namespace countdown::platform
