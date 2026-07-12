#include "main_window.hpp"

#include "platform/platform.hpp"

#include <countdown/version.hpp>

#include <QApplication>
#include <QString>

#include <cstdio>
#include <string_view>

namespace {

[[nodiscard]] QString version_qstring() {
    return QString::fromUtf8(countdown::version_string.data(),
                             static_cast<qsizetype>(countdown::version_string.size()));
}

}  // namespace

int main(int argc, char* argv[]) {
    // Handle informational flags before starting Qt so they work headlessly.
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--version" || arg == "-v") {
            countdown::platform::ensure_console_output();
            std::printf("%s\n", countdown::version_details().c_str());
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            countdown::platform::ensure_console_output();
            std::printf("Usage: countdown_app [--version] [--help]\n");
            return 0;
        }
    }

    QApplication application(argc, argv);
    QApplication::setApplicationName(QStringLiteral("CountdownSolver"));
    QApplication::setOrganizationName(QStringLiteral("CountdownSolver"));
    QApplication::setApplicationVersion(version_qstring());

    countdown::app::MainWindow window;
    window.show();
    return QApplication::exec();
}
