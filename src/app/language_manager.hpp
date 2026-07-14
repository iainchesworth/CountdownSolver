#pragma once

#include <QGuiApplication>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QTranslator>
#include <QVariantList>

namespace countdown::app {

// Owns the app's QTranslators (Qt's own base strings plus the countdownsolver
// UI strings), the persisted language preference, and everything a language
// switch touches: installed translators, the application's layout direction
// (LTR/RTL), and QQmlEngine::retranslate(). No game logic lives here - Solver
// stays purely about Numbers/Letters/Conundrum.
//
// Takes QQmlEngine& rather than QQmlApplicationEngine& - retranslate() is
// declared directly on QQmlEngine (Qt 6.7+) and every real caller either has
// one already (main.cpp's QQmlApplicationEngine) or only has a QQmlEngine to
// begin with (the Quick Test harness's qmlEngineAvailable(QQmlEngine*) hook,
// see tests/qml/tst_qml_setup.hpp) - accepting the base type lets both
// construct a real LanguageManager instead of a test-only stub.
class LanguageManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)

public:
    // `app` and `engine` must outlive this LanguageManager.
    explicit LanguageManager(QGuiApplication& app, QQmlEngine& engine, QObject* parent = nullptr);

    // Installs the translators for the initial language - the persisted
    // preference, falling back to the system locale (and, failing that,
    // "en") - and sets the initial layout direction. Called once from
    // main(), before the QML is loaded, so the first frame already renders
    // translated and RTL-mirrored where appropriate.
    void applyInitialLanguage();

    [[nodiscard]] QString currentLanguage() const noexcept { return current_language_; }

    // One { "code": QString, "name": QString } entry per supported language,
    // "en" (no .qm - falls back to the untranslated source strings) first.
    // Names are each language's own native name, so a user can find their
    // language regardless of whatever language the UI is currently in.
    Q_INVOKABLE QVariantList availableLanguages() const;

    // Persists `code`, swaps the installed translators, updates the
    // application's layout direction, and retranslates the running QML.
    // Returns false (no change made) if `code` isn't one of
    // availableLanguages()'s codes.
    Q_INVOKABLE bool setLanguage(const QString& code);

signals:
    void currentLanguageChanged();

private:
    void installTranslators(const QString& code);

    QGuiApplication& app_;
    QQmlEngine& engine_;
    QTranslator qt_translator_;
    QTranslator app_translator_;
    QString current_language_ = QStringLiteral("en");
};

}  // namespace countdown::app
