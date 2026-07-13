#pragma once

#include "solver.hpp"

#include <QObject>

// Solver::versionDetails() and Solver::shortVersion(): the About/Settings
// version display contracts.
class VersionTest : public QObject {
    Q_OBJECT

public:
    // `solver` is owned by app_tests_main.cpp and shared across every test
    // class that only needs a default-dictionary Solver - see the comment
    // there for why.
    explicit VersionTest(countdown::app::Solver& solver) : solver_(solver) {}

private slots:
    void versionDetails_nonEmpty();
    void shortVersion_matchesLibraryVersion();

private:
    countdown::app::Solver& solver_;
};
