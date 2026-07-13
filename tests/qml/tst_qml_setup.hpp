#pragma once

#include "language_manager.hpp"
#include "solver.hpp"

#include <QObject>
#include <QQmlEngine>

#include <optional>

// Registers the same `solver`/`languageManager` context properties main.cpp
// installs for the real app, so the TestCase QML files under tests/qml/ can
// drive NumbersPage/LettersPage/ConundrumPage/SettingsPage exactly as the app
// does - including Theme.qml's languageManager.currentLanguage binding and
// SettingsPage.qml's language picker.
class Setup : public QObject {
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine* engine);

private:
    countdown::app::Solver solver_;

    // Constructed lazily in qmlEngineAvailable() rather than as a plain
    // member: LanguageManager holds reference members (to the application
    // and the engine), neither of which exists yet when Setup itself is
    // constructed.
    std::optional<countdown::app::LanguageManager> language_manager_;
};
