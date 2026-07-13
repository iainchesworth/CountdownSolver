#include "solver.hpp"

#include "logging/logging.hpp"
#include "platform/platform.hpp"

#include <countdown/version.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
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
    // Installed before anything else so every subsequent qCDebug/qCWarning/...
    // call (including ones triggered by Qt/QML internals during startup) is
    // captured, not just those after QGuiApplication is constructed.
    countdown::app::logging::install();

    // Every custom control (FlatButton, PadButton, ...) customizes `background`/
    // `contentItem`; native styles (the default on Windows) silently ignore that
    // customization and fall back to OS-drawn chrome. "Basic" is the only style
    // that honors it everywhere, so Switch/Slider/etc. render as designed too.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

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

    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &app,
                      [](const QList<QQmlError>& warnings) {
                          for (const QQmlError& warning : warnings) {
                              qCWarning(lcQml) << warning.toString();
                          }
                      });

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [](const QUrl& url) {
            qCCritical(lcQml) << "failed to create QML root object from" << url;
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.loadFromModule("Countdown", "Main");

    return QGuiApplication::exec();
}
