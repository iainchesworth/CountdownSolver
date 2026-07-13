#pragma once

#include "solver.hpp"

#include <QObject>

class SolverTest : public QObject {
    Q_OBJECT

private slots:
    void solveNumbers_exact();
    void solveNumbers_closestFallback();
    void solveNumbers_emptyInput();

    void solveLetters_groupsByLength();
    void solveLetters_respectsMaxResults();
    void solveLetters_noMatches();

    void solveConundrum_found();
    void solveConundrum_notFound();

    void randomRack_respectsWeightedTilePool();
    void randomConundrum_returnsNineLetters();

    void versionDetails_nonEmpty();
    void shortVersion_matchesLibraryVersion();

    void dictionaryWordCount_matchesActiveDictionary();

    void fullDictionary_defaultInvariant();
#ifdef Q_OS_LINUX
    void fullDictionary_foundOnDisk();
#endif

private:
    // Built once for the whole test binary and reused by every slot above
    // that only needs a default-dictionary Solver (mirrors Setup::solver_ in
    // tests/qml/tst_qml_setup.hpp) - constructing one re-parses, sorts and
    // builds a frequency table for the full bundled dictionary, which is
    // measurably expensive under Debug+ASan, so doing it once instead of
    // once per slot cuts this binary's runtime dramatically. Slots that test
    // construction-time behaviour itself (full-dictionary discovery/
    // invariants) still build their own local Solver - see the slots' own
    // definitions.
    countdown::app::Solver solver_;
};
