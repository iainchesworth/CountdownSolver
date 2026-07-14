#include "tst_qml_setup.hpp"

#include <QGuiApplication>
#include <QQmlContext>
#include <QString>
#include <QtQuickTest/quicktest.h>

void Setup::qmlEngineAvailable(QQmlEngine* engine) {
    engine->rootContext()->setContextProperty(QStringLiteral("solver"), &solver_);

    // qGuiApp: Quick Test's own main() already constructed the process's one
    // QGuiApplication before this hook runs.
    language_manager_.emplace(*qGuiApp, *engine);
    language_manager_->applyInitialLanguage();
    engine->rootContext()->setContextProperty(QStringLiteral("languageManager"),
                                               &*language_manager_);

    // Mirrors main.cpp's solver<->languageManager wiring so a language
    // switch reloads the dictionary here exactly as it does in the real app
    // (see tst_LanguageSwitching.qml, which switches language repeatedly).
    solver_.setLanguageCode(language_manager_->currentLanguage());
    QObject::connect(&*language_manager_, &countdown::app::LanguageManager::currentLanguageChanged,
                      &solver_, [this]() { solver_.setLanguageCode(language_manager_->currentLanguage()); });
}

QUICK_TEST_MAIN_WITH_SETUP(countdown_qml, Setup)
