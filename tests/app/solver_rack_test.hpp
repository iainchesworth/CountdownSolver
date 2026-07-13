#pragma once

#include "solver.hpp"

#include <QObject>

// Solver::randomRack() and Solver::randomConundrum(): the letter-generation
// contracts, independent of solveLetters()/solveConundrum() themselves.
class RackGenerationTest : public QObject {
    Q_OBJECT

public:
    // `solver` is owned by app_tests_main.cpp and shared across every test
    // class that only needs a default-dictionary Solver - see the comment
    // there for why.
    explicit RackGenerationTest(countdown::app::Solver& solver) : solver_(solver) {}

private slots:
    void randomRack_respectsWeightedTilePool();
    void randomConundrum_returnsNineLetters();

private:
    countdown::app::Solver& solver_;
};
