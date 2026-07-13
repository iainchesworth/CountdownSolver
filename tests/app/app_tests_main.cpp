#include "solver.hpp"

#include "solver_conundrum_test.hpp"
#include "solver_dictionary_test.hpp"
#include "solver_letters_test.hpp"
#include "solver_logging_test.hpp"
#include "solver_numbers_test.hpp"
#include "solver_rack_test.hpp"
#include "solver_version_test.hpp"

#include <QTest>

// countdown_app_tests bundles several one-class-per-concern QtTest suites
// (see the split rationale in tests/CMakeLists.txt's countdown_app_tests
// comment). QTEST_APPLESS_MAIN only wires up a single QObject-derived class,
// so with several classes in one binary a hand-written main() aggregating
// them via QTest::qExec() - the pattern Qt's own docs recommend for multiple
// test classes in one executable - takes its place. No QCoreApplication is
// constructed here, matching what QTEST_APPLESS_MAIN would have done.
int main(int argc, char* argv[]) {
    int status = 0;

    // Built once and shared (by reference) across every test class that only
    // needs a default-dictionary Solver: constructing one re-parses, sorts
    // and builds a frequency table for the full bundled dictionary, which is
    // measurably expensive under Debug+ASan, so doing it once instead of
    // once per test class cuts this binary's runtime dramatically. Classes
    // that test construction-time behaviour itself (full-dictionary
    // discovery/invariants) still build their own local Solver - see
    // DictionaryTest.
    countdown::app::Solver solver;

    {
        NumbersSolverTest test(solver);
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        LettersSolverTest test(solver);
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        ConundrumSolverTest test(solver);
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        RackGenerationTest test(solver);
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        VersionTest test(solver);
        status |= QTest::qExec(&test, argc, argv);
    }
    {
        DictionaryTest test;
        status |= QTest::qExec(&test, argc, argv);
    }
#ifdef Q_OS_LINUX
    {
        LoggingTest test;
        status |= QTest::qExec(&test, argc, argv);
    }
#endif

    return status;
}
