#pragma once

#include "solver.hpp"

#include <QObject>

// Solver::solveNumbers(): exact hits, the closest-fallback branch, and the
// empty-input edge case.
class NumbersSolverTest : public QObject {
    Q_OBJECT

public:
    // `solver` is owned by app_tests_main.cpp and shared across every test
    // class that only needs a default-dictionary Solver - see the comment
    // there for why.
    explicit NumbersSolverTest(countdown::app::Solver& solver) : solver_(solver) {}

private slots:
    void solveNumbers_exact();
    void solveNumbers_closestFallback();
    void solveNumbers_emptyInput();

private:
    countdown::app::Solver& solver_;
};
