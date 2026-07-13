#pragma once

#include "solver.hpp"

#include <QObject>
#include <QQmlEngine>

// Registers the same `solver` context property that main.cpp installs for
// the real app, so the TestCase QML files under tests/qml/ can drive
// NumbersPage/LettersPage/ConundrumPage/SettingsPage exactly as the app does.
class Setup : public QObject {
    Q_OBJECT

public slots:
    void qmlEngineAvailable(QQmlEngine* engine);

private:
    countdown::app::Solver solver_;
};
