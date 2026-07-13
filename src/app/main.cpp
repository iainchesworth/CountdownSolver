#include "solver.hpp"

#include "logging/logging.hpp"
#include "platform/platform.hpp"

#include <countdown/version.hpp>

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QString>
#include <QTimer>
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

    // Dictionary loading is deferred (see Solver::loadDictionaries below)
    // rather than done here in the constructor, so parsing/indexing the
    // ~1.1MB bundled word list doesn't block the window from appearing.
    countdown::app::Solver solver(nullptr, countdown::app::Solver::DictionaryLoad::kDeferred);

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

    // QQuickWindow::setIcon() has no QML-facing equivalent (unlike QWindow,
    // QQuickWindow never grew an `icon` property - see QTBUG-31601), so the
    // window/taskbar icon has to be set here in C++ rather than in Main.qml.
    // This is separate from the Windows .rc/.ico (src/app/CMakeLists.txt),
    // which only covers the exe file's own icon in Explorer/Alt-Tab before
    // any window exists; this covers the actual running window on every
    // platform, including the ones without an embeddable exe icon resource.
    if (!engine.rootObjects().isEmpty()) {
        if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().constFirst())) {
            window->setIcon(QIcon(QStringLiteral(":/icon/app_256.png")));
        }
    }

    // Runs as soon as the event loop starts processing, i.e. once the window
    // has already been created and its first frame queued, rather than
    // blocking every launch on dictionary I/O before anything is shown.
    QTimer::singleShot(0, &solver, &countdown::app::Solver::loadDictionaries);

    return QGuiApplication::exec();
}
