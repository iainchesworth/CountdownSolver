#include "platform/platform.hpp"

#include <cstdlib>
#include <filesystem>
#include <utility>

namespace countdown::platform {

PlatformInfo current() {
    // Follow the XDG Base Directory spec: $XDG_CONFIG_HOME, else ~/.config.
    std::filesystem::path config_dir;
    if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg != nullptr && *xdg != '\0') {
        config_dir = std::filesystem::path(xdg) / "countdown-solver";
    } else if (const char* home = std::getenv("HOME")) {
        config_dir = std::filesystem::path(home) / ".config" / "countdown-solver";
    }
    return PlatformInfo{"Linux", std::move(config_dir)};
}

void ensure_console_output() {
    // On Linux the process already inherits the launching terminal's stdout.
}

}  // namespace countdown::platform
