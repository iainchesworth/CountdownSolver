#include "platform/platform.hpp"

#include <QStandardPaths>
#include <QString>

#include <filesystem>
#include <utility>

namespace countdown::platform {

PlatformInfo current() {
    // Sandboxed apps have no $HOME-style env var; QStandardPaths wraps the
    // real per-app-container config location (NSSearchPathForDirectoriesInDomains
    // under the hood).
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    std::filesystem::path config_dir;
    if (!dir.isEmpty()) {
        config_dir = std::filesystem::path(dir.toStdString());
    }
    return PlatformInfo{"iOS", std::move(config_dir)};
}

void ensure_console_output() {
    // Launched by the OS with no argv/controlling terminal; --version/--help
    // are unreachable here, same no-op as Linux/macOS.
}

}  // namespace countdown::platform
