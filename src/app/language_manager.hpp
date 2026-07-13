#pragma once

#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QString>
#include <QTranslator>
#include <QVariantList>

namespace countdown::app {

// Owns the app's QTranslators (Qt's own base strings plus the countdown_app
// UI strings), the persisted language preference, and everything a language
// switch touches: installed translators, the application's layout direction
// (LTR/RTL), and QQmlApplicationEngine::retranslate(). No game logic lives
// here - Solver stays purely about Numbers/Letters/Conundrum.
class LanguageManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)

public:
    // `app` and `engine` must outlive this LanguageManager.
    explicit LanguageManager(QGuiApplication& app, QQmlApplicationEngine& engine,
                              QObject* parent = nullptr);

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
    QQmlApplicationEngine& engine_;
    QTranslator qt_translator_;
    QTranslator app_translator_;
    QString current_language_ = QStringLiteral("en");
};

}  // namespace countdown::app
