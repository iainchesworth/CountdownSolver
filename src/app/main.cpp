#include "language_manager.hpp"
#include "solver.hpp"

#include "logging/logging.hpp"
#include "platform/platform.hpp"

#include <countdown/version.hpp>

#include <QFontDatabase>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QString>
#include <QTimer>

#include <cstdio>
#include <optional>
#include <string_view>

namespace {

[[nodiscard]] QString version_qstring() {
    return QString::fromUtf8(countdown::version_string.data(),
                             static_cast<qsizetype>(countdown::version_string.size()));
}

}  // namespace

int main(int argc, char* argv[]) {
    // Set before logging::install() below: on Android/iOS, platform::current()
    // resolves the config directory via QStandardPaths::AppConfigLocation,
    // which is built from these names. Both are static setters safely callable
    // before QGuiApplication is constructed.
    QGuiApplication::setApplicationName(QStringLiteral("Countdown Solver"));
    QGuiApplication::setOrganizationName(QStringLiteral("CountdownSolver"));

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
            std::printf("Usage: countdownsolver [--version] [--help]\n");
            return 0;
        }
    }

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationVersion(version_qstring());

    // Arabic/Hebrew/Yiddish glyph coverage - IBM Plex Sans (Theme.qml's
    // default) has none. Theme.qml switches to these by name once a language
    // needing them is active (see LanguageManager). Both are variable fonts
    // (a weight axis); Qt registers each under its single family name
    // ("Noto Sans Arabic"/"Noto Sans Hebrew") regardless.
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansArabic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansHebrew.ttf"));

    // Dictionary loading is deferred (see Solver::loadDictionaries below)
    // rather than done here in the constructor, so parsing/indexing the
    // ~1.1MB bundled word list doesn't block the window from appearing.
    countdown::app::Solver solver(nullptr, countdown::app::Solver::DictionaryLoad::kDeferred);

    // Held in an optional (rather than a plain stack object) so it can be
    // destroyed explicitly, before main() returns, rather than via the
    // default stack-unwind order. solver/language_manager are declared
    // below and would otherwise be destroyed *before* engine (C++ destroys
    // locals in reverse declaration order) - but engine's QML tree holds
    // both as context properties, and Qt nulls out a QML reference the
    // moment the underlying QObject is destroyed. Any binding still being
    // evaluated while the engine itself tears down (Theme.qml's singleton,
    // SettingsPage.qml, LetterRackInput.qml all read languageManager) would
    // then see it as null and throw - reproduced on every close of a real
    // build, not just intermittently. Destroying engine first (see below)
    // ensures the whole QML tree is gone before solver/language_manager are.
    std::optional<QQmlApplicationEngine> engine{std::in_place};

    // Installs translators and sets the initial layout direction before any
    // QML is loaded (loadFromModule() below), so the first frame already
    // renders translated and RTL-mirrored where appropriate.
    countdown::app::LanguageManager language_manager(app, *engine);
    language_manager.applyInitialLanguage();

    // Cheap - dictionaries haven't loaded yet (see the deferred
    // loadDictionaries() call below), so this only records which alphabet/
    // word list the first load should use; every subsequent language
    // switch reloads immediately via this same connection.
    solver.setLanguageCode(language_manager.currentLanguage());
    QObject::connect(&language_manager, &countdown::app::LanguageManager::currentLanguageChanged,
                      &solver, [&language_manager, &solver]() {
                          solver.setLanguageCode(language_manager.currentLanguage());
                      });

    engine->rootContext()->setContextProperty(QStringLiteral("solver"), &solver);
    engine->rootContext()->setContextProperty(QStringLiteral("languageManager"), &language_manager);

    QObject::connect(&*engine, &QQmlApplicationEngine::warnings, &app,
                      [](const QList<QQmlError>& warnings) {
                          for (const QQmlError& warning : warnings) {
                              qCWarning(lcQml) << warning.toString();
                          }
                      });

    QObject::connect(
        &*engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [](const QUrl& url) {
            qCCritical(lcQml) << "failed to create QML root object from" << url;
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine->loadFromModule("Countdown", "Main");

    // QQuickWindow::setIcon() has no QML-facing equivalent (unlike QWindow,
    // QQuickWindow never grew an `icon` property - see QTBUG-31601), so the
    // window/taskbar icon has to be set here in C++ rather than in Main.qml.
    // This is separate from the Windows .rc/.ico (src/app/CMakeLists.txt),
    // which only covers the exe file's own icon in Explorer/Alt-Tab before
    // any window exists; this covers the actual running window on every
    // platform, including the ones without an embeddable exe icon resource.
    if (!engine->rootObjects().isEmpty()) {
        if (auto* window = qobject_cast<QQuickWindow*>(engine->rootObjects().constFirst())) {
            window->setIcon(QIcon(QStringLiteral(":/icon/app_256.png")));
        }
    }

    // Runs as soon as the event loop starts processing, i.e. once the window
    // has already been created and its first frame queued, rather than
    // blocking every launch on dictionary I/O before anything is shown.
    QTimer::singleShot(0, &solver, &countdown::app::Solver::loadDictionaries);

    const int exit_code = QGuiApplication::exec();

    // Explicitly torn down here - see the comment on `engine`'s declaration
    // above - while solver/language_manager (destroyed below, via the
    // ordinary end-of-scope unwind) are still alive to be read by any
    // binding evaluated during this teardown.
    engine.reset();

    return exit_code;
}
