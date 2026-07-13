#pragma once

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

    void randomRack_returnsARealFallbackWordsLetters();
    void randomConundrum_returnsNineLetters();

    void versionDetails_nonEmpty();
    void shortVersion_matchesLibraryVersion();

    void fullDictionary_defaultInvariant();
#ifdef Q_OS_LINUX
    void fullDictionary_foundOnDisk();
#endif
};
