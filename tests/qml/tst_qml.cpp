#include "tst_qml_setup.hpp"

#include <QAccessible>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QMetaEnum>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSettings>
#include <QString>
#include <QTest>
#include <QWindow>
#include <QtQuickTest/quicktest.h>

#include <memory>
#include <optional>

QVariant RestartHarness::afterRestart(const QString& expression) const {
    // A LanguageManager changes process-wide state outside itself (the
    // application's layout direction and the default QLocale). Both are put
    // back afterwards so the test that asked for a "restart" carries on in the
    // state it was in, not the one the restarted app settled into.
    const QLocale saved_locale;
    const Qt::LayoutDirection saved_direction = qGuiApp->layoutDirection();

    QVariant result;
    {
        countdown::app::Preferences preferences;

        // Held in an optional so it can be torn down explicitly, *before*
        // language_manager/preferences, for the reason main.cpp gives for the
        // real engine: the QML tree reads both as context properties, and
        // would see them as null mid-teardown if they died first.
        std::optional<QQmlEngine> engine{std::in_place};
        countdown::app::LanguageManager language_manager(*qGuiApp, *engine);
        language_manager.applyInitialLanguage();
        engine->rootContext()->setContextProperty(QStringLiteral("preferences"), &preferences);
        engine->rootContext()->setContextProperty(QStringLiteral("languageManager"),
                                                  &language_manager);

        {
            QQmlComponent component(&*engine);
            const QString source = QStringLiteral("import QtQuick\nimport Countdown\n") +
                                   QStringLiteral("QtObject { property var value: ") + expression +
                                   QStringLiteral(" }");
            component.setData(source.toUtf8(), QUrl());
            const std::unique_ptr<QObject> object{component.create()};
            if (object) {
                result = object->property("value");
            } else {
                qWarning() << "RestartHarness::afterRestart failed:" << component.errors();
            }
        }

        engine.reset();
    }

    QLocale::setDefault(saved_locale);
    qGuiApp->setLayoutDirection(saved_direction);
    return result;
}

QVariantMap AccessibilityProbe::info(QObject* item) const {
    QVariantMap result;
    QAccessibleInterface* const accessible = QAccessible::queryAccessibleInterface(item);
    if (accessible == nullptr) {
        return result;
    }

    const QAccessible::State state = accessible->state();
    result[QStringLiteral("role")] = QString::fromLatin1(
        QMetaEnum::fromType<QAccessible::Role>().valueToKey(static_cast<int>(accessible->role())));
    result[QStringLiteral("name")] = accessible->text(QAccessible::Name);
    result[QStringLiteral("description")] = accessible->text(QAccessible::Description);
    result[QStringLiteral("checked")] = static_cast<bool>(state.checked);
    result[QStringLiteral("selected")] = static_cast<bool>(state.selected);
    result[QStringLiteral("focusable")] = static_cast<bool>(state.focusable);
    result[QStringLiteral("disabled")] = static_cast<bool>(state.disabled);
    return result;
}

void AccessibilityProbe::keyClick(QObject* window, int key, int modifiers) const {
    auto* const target = qobject_cast<QWindow*>(window);
    if (target == nullptr) {
        qWarning() << "AccessibilityProbe::keyClick: not a window:" << window;
        return;
    }
    QTest::keyClick(target, static_cast<Qt::Key>(key), static_cast<Qt::KeyboardModifiers>(modifiers));
}

Setup::Setup() {
    // The style main.cpp selects: every custom control customizes
    // `background`/`contentItem`, which native styles (the default on
    // Windows) ignore - so without this the tests would exercise a different
    // control tree, and different focus behaviour, than the app ships.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QCoreApplication::setOrganizationName(QStringLiteral("CountdownSolverTests"));
    QCoreApplication::setApplicationName(QStringLiteral("countdown_qml_tests"));

    // INI, not the native format: on Windows the native store is the registry.
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings_dir_.path());
}

void Setup::qmlEngineAvailable(QQmlEngine* engine) {
    // Every test process starts as English, wherever it runs. Without this the
    // first language is whatever the host's system locale maps to, and the
    // assertions that start from "en" would depend on the machine. (Done here,
    // not in the constructor: QSettings is only safe to write once the
    // application object exists.)
    {
        QSettings settings;
        settings.setValue(QStringLiteral("language/code"), QStringLiteral("en"));
    }

    engine->rootContext()->setContextProperty(QStringLiteral("solver"), &solver_);
    engine->rootContext()->setContextProperty(QStringLiteral("appRestart"), &restart_);
    engine->rootContext()->setContextProperty(QStringLiteral("a11y"), &probe_);

    // qGuiApp: Quick Test's own main() already constructed the process's one
    // QGuiApplication before this hook runs.
    language_manager_.emplace(*qGuiApp, *engine);
    language_manager_->applyInitialLanguage();
    engine->rootContext()->setContextProperty(QStringLiteral("languageManager"),
                                               &*language_manager_);

    preferences_.emplace();
    engine->rootContext()->setContextProperty(QStringLiteral("preferences"), &*preferences_);

    // Mirrors main.cpp's solver<->languageManager wiring so a language
    // switch reloads the dictionary here exactly as it does in the real app
    // (see tst_LanguageSwitching.qml, which switches language repeatedly).
    solver_.setLanguageCode(language_manager_->currentLanguage());
    QObject::connect(&*language_manager_, &countdown::app::LanguageManager::currentLanguageChanged,
                      &solver_, [this]() { solver_.setLanguageCode(language_manager_->currentLanguage()); });
}

QUICK_TEST_MAIN_WITH_SETUP(countdown_qml, Setup)
