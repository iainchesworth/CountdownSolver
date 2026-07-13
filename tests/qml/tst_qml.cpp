#include "tst_qml_setup.hpp"

#include <QQmlContext>
#include <QString>
#include <QtQuickTest/quicktest.h>

void Setup::qmlEngineAvailable(QQmlEngine* engine) {
    engine->rootContext()->setContextProperty(QStringLiteral("solver"), &solver_);
}

QUICK_TEST_MAIN_WITH_SETUP(countdown_qml, Setup)
