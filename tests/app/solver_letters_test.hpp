#pragma once

#include "solver.hpp"

#include <QObject>

// Solver::solveLetters(): grouping/ordering of matches, the maxResults cap,
// and the no-matches edge case.
class LettersSolverTest : public QObject {
    Q_OBJECT

public:
    // `solver` is owned by app_tests_main.cpp and shared across every test
    // class that only needs a default-dictionary Solver - see the comment
    // there for why.
    explicit LettersSolverTest(countdown::app::Solver& solver) : solver_(solver) {}

private slots:
    void solveLetters_groupsByLength();
    void solveLetters_respectsMaxResults();
    void solveLetters_noMatches();

private:
    countdown::app::Solver& solver_;
};
