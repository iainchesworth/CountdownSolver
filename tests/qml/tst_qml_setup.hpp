#pragma once

#include "language_manager.hpp"
#include "preferences.hpp"
#include "solver.hpp"

#include <QObject>
#include <QQmlEngine>
#include <QTemporaryDir>
#include <QVariant>
#include <QVariantMap>

#include <optional>

// Stands in for a relaunch of the app: boots a second, throwaway QQmlEngine
// wired the way main.cpp wires the real one (a fresh Preferences and
// LanguageManager that read whatever QSettings holds *now*), evaluates one QML
// expression against it, and tears it down. Theme and AppState are
// per-engine singletons, so the fresh engine gets fresh ones seeded from
// storage rather than the live objects the running test has been mutating -
// exactly what a restart does. Exposed to QML as `appRestart`; see
// tst_ThemePersistence.qml / tst_LanguageSwitching.qml.
class RestartHarness : public QObject {
    Q_OBJECT

public:
    // `expression` is evaluated with `import QtQuick` and `import Countdown`
    // in scope; its value is returned (an invalid QVariant if the expression
    // fails to compile or the engine fails to start, after logging why).
    Q_INVOKABLE QVariant afterRestart(const QString& expression) const;
};

// What assistive technology would be told about an item: the role and name a
// screen reader announces, via the same QAccessibleInterface it queries. QML
// can't read another item's `Accessible.*` attached properties, so the
// accessibility tests go through this instead. Exposed to QML as `a11y`; see
// tst_Accessibility.qml.
class AccessibilityProbe : public QObject {
    Q_OBJECT

public:
    // { role, name, description, checked, selected, focusable, disabled } for
    // `item`, or an empty map if it is not exposed to assistive technology at
    // all. `role` is the QAccessible::Role enumerator's name ("Button",
    // "RadioButton", "PageTab", "StaticText", ...).
    Q_INVOKABLE QVariantMap info(QObject* item) const;

    // Types `key` (a Qt.Key_* value, with optional Qt.*Modifier bits) into
    // `window`: a press then a release, as a real keyboard would. QML's own
    // keyClick() can only reach the window the TestCase lives in; this reaches
    // any top-level window, which is what driving the real Main shell needs.
    Q_INVOKABLE void keyClick(QObject* window, int key, int modifiers = 0) const;
};

// Registers the same `solver`/`languageManager`/`preferences` context
// properties main.cpp installs for the real app, so the TestCase QML files
// under tests/qml/ can drive NumbersPage/LettersPage/ConundrumPage/
// SettingsPage exactly as the app does - including Theme.qml's
// languageManager.currentLanguage and preferences.theme bindings and
// SettingsPage.qml's language picker.
class Setup : public QObject {
    Q_OBJECT

public:
    // Runs before QGuiApplication exists (QUICK_TEST_MAIN_WITH_SETUP builds
    // this first), so it can only do static QSettings/application-name
    // configuration - nothing that reads or writes settings.
    Setup();

public slots:
    void qmlEngineAvailable(QQmlEngine* engine);

private:
    // Points QSettings at a private INI file for the life of the process, so
    // the preferences these tests write (and the restarts they simulate)
    // never touch the developer's or CI runner's real registry/config store.
    // Declared first: it must outlive every object that writes settings.
    QTemporaryDir settings_dir_;

    countdown::app::Solver solver_;

    // Constructed lazily in qmlEngineAvailable() rather than as plain
    // members: LanguageManager holds reference members (to the application
    // and the engine), neither of which exists yet when Setup itself is
    // constructed, and Preferences reads QSettings at construction, which
    // isn't configured until Setup's own constructor body has run.
    std::optional<countdown::app::LanguageManager> language_manager_;
    std::optional<countdown::app::Preferences> preferences_;
    RestartHarness restart_;
    AccessibilityProbe probe_;
};
