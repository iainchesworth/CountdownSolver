#pragma once

#include "solver.hpp"

#include <QObject>

// Solver::solveConundrum(): a found single answer, the not-found case, and
// the empty-input edge case.
class ConundrumSolverTest : public QObject {
    Q_OBJECT

public:
    // `solver` is owned by app_tests_main.cpp and shared across every test
    // class that only needs a default-dictionary Solver - see the comment
    // there for why.
    explicit ConundrumSolverTest(countdown::app::Solver& solver) : solver_(solver) {}

private slots:
    void solveConundrum_found();
    void solveConundrum_notFound();
    void solveConundrum_emptyInput();

private:
    countdown::app::Solver& solver_;
};
