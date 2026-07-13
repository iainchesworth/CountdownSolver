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
}

QUICK_TEST_MAIN_WITH_SETUP(countdown_qml, Setup)
