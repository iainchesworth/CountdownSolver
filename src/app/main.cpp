#include "solver.hpp"

#include "platform/platform.hpp"

#include <countdown/version.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QString>
// #include <QFontDatabase>  // uncomment if bundling IBM Plex (see README)

#include <cstdio>
#include <string_view>

namespace {

[[nodiscard]] QString version_qstring() {
    return QString::fromUtf8(countdown::version_string.data(),
                             static_cast<qsizetype>(countdown::version_string.size()));
}

}  // namespace

int main(int argc, char* argv[]) {
    // Informational flags are handled before Qt starts so they work headlessly.
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

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Countdown Solver"));
    QGuiApplication::setOrganizationName(QStringLiteral("CountdownSolver"));
    QGuiApplication::setApplicationVersion(version_qstring());

    // --- optional: bundle the design's fonts for identical metrics everywhere ---
    // QFontDatabase::addApplicationFont(":/fonts/IBMPlexSans-Regular.ttf");
    // QFontDatabase::addApplicationFont(":/fonts/IBMPlexMono-Regular.ttf");

    countdown::app::Solver solver;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("solver"), &solver);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("Countdown", "Main");

    return QGuiApplication::exec();
}
