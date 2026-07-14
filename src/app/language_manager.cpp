#include "language_manager.hpp"

#include "logging/logging.hpp"

#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QVariantMap>

#include <array>

namespace countdown::app {
namespace {

struct LanguageInfo {
    const char* code;
    const char* nativeName;
};

// "en" first (no .qm - the untranslated source strings are already English),
// then every language this app ships translations for (see
// src/app/translations/). Names are UTF-8 literals in each language's own
// script, not the current UI language, so a user can recognise their entry
// regardless of what the UI currently shows.
constexpr std::array<LanguageInfo, 7> kLanguages{{
    {"en", "English"},
    {"fr", "Français"},
    {"de", "Deutsch"},
    {"es", "Español"},
    {"ar", "العربية"},
    {"he", "עברית"},
    {"yi", "יידיש"},
}};

constexpr auto kSettingsKey = "language/code";

[[nodiscard]] bool is_supported(const QString& code) {
    for (const LanguageInfo& info : kLanguages) {
        if (code == QLatin1String(info.code)) {
            return true;
        }
    }
    return false;
}

// Maps a system locale to one of kLanguages, or "en" if unsupported - so an
// unrecognised system locale falls back to the untranslated UI rather than
// failing to find any translator at all.
[[nodiscard]] QString language_for_locale(const QLocale& locale) {
    const QString name = QLocale::languageToCode(locale.language());
    return is_supported(name) ? name : QStringLiteral("en");
}

}  // namespace

LanguageManager::LanguageManager(QGuiApplication& app, QQmlEngine& engine, QObject* parent)
    : QObject(parent), app_(app), engine_(engine) {}

void LanguageManager::applyInitialLanguage() {
    const QSettings settings;
    const QString saved = settings.value(QLatin1String(kSettingsKey)).toString();
    const QString initial = is_supported(saved) ? saved : language_for_locale(QLocale::system());

    installTranslators(initial);
    app_.setLayoutDirection(QLocale(initial).textDirection());
    current_language_ = initial;
}

QVariantList LanguageManager::availableLanguages() const {
    QVariantList result;
    result.reserve(static_cast<qsizetype>(kLanguages.size()));
    for (const LanguageInfo& info : kLanguages) {
        QVariantMap entry;
        entry["code"] = QString::fromLatin1(info.code);
        entry["name"] = QString::fromUtf8(info.nativeName);
        result << entry;
    }
    return result;
}

bool LanguageManager::setLanguage(const QString& code) {
    if (!is_supported(code)) {
        qCWarning(lcApp) << "setLanguage: unsupported language code" << code;
        return false;
    }
    if (code == current_language_) {
        return true;
    }

    installTranslators(code);
    app_.setLayoutDirection(QLocale(code).textDirection());
    // current_language_ must be updated before retranslate(): retranslate()
    // synchronously re-evaluates every live qsTr()-using binding, and any of
    // those that also read currentLanguage() (e.g. SettingsPage.qml's
    // dictionary-status text) must see the new code, not the language being
    // switched away from.
    current_language_ = code;
    engine_.retranslate();

    QSettings settings;
    settings.setValue(QLatin1String(kSettingsKey), code);

    emit currentLanguageChanged();
    return true;
}

void LanguageManager::installTranslators(const QString& code) {
    // Each translator's previous language (if any) is replaced in place via
    // QTranslator::load() rather than swapped for a fresh instance, so a
    // failed load below cleanly leaves the old translator removed instead of
    // half-installed.
    app_.removeTranslator(&qt_translator_);
    app_.removeTranslator(&app_translator_);

    if (code == QLatin1String("en")) {
        return;  // No translator needed - source strings are already English.
    }

    // Qt 6 consolidated its own translation catalogs (formerly qtbase_xx,
    // qtdeclarative_xx, ...) into a single qt_xx.qm per language.
    if (qt_translator_.load(QLocale(code), QStringLiteral("qt"), QStringLiteral("_"),
                             QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app_.installTranslator(&qt_translator_);
    } else {
        qCDebug(lcApp) << "installTranslators: no bundled Qt translation for" << code;
    }

    if (app_translator_.load(QLocale(code), QStringLiteral("countdown"), QStringLiteral("_"),
                              QStringLiteral(":/i18n"))) {
        app_.installTranslator(&app_translator_);
    } else {
        qCWarning(lcApp) << "installTranslators: no countdown_" << code << ".qm resource found";
    }
}

}  // namespace countdown::app
