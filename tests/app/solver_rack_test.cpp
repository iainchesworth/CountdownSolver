#include "solver_rack_test.hpp"

#include <QString>
#include <QTest>
#include <QVariantMap>

#include <algorithm>
#include <array>
#include <utility>

namespace {

// Legal vowel/consonant splits for a 9-letter rack, mirroring
// Solver::randomRack()'s kVowelConsonantSplits (src/app/solver.cpp) - not
// exposed publicly, so duplicated here to check the contract from outside.
constexpr std::array<std::pair<int, int>, 3> kLegalSplits{{{3, 6}, {4, 5}, {5, 4}}};

[[nodiscard]] bool isVowel(QChar ch) {
    switch (ch.toLower().unicode()) {
        case 'a': case 'e': case 'i': case 'o': case 'u': return true;
        default: return false;
    }
}

}  // namespace

void RackGenerationTest::randomRack_respectsWeightedTilePool() {
    const QString rack = solver_.randomRack();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());

    const int vowels = static_cast<int>(std::count_if(rack.begin(), rack.end(), isVowel));
    const int consonants = static_cast<int>(rack.size()) - vowels;
    const bool isLegalSplit = std::any_of(
        kLegalSplits.begin(), kLegalSplits.end(),
        [&](const auto& split) { return split.first == vowels && split.second == consonants; });
    QVERIFY(isLegalSplit);
}

void RackGenerationTest::randomConundrum_returnsNineLetters() {
    const QString rack = solver_.randomConundrum();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());

    // randomConundrum shuffles a real dictionary word, so solving it back
    // must always find at least that word again.
    const QVariantMap result = solver_.solveConundrum(rack);
    QCOMPARE(result["found"].toBool(), true);
}

void RackGenerationTest::randomRack_respectsFrenchRackSize() {
    // A dedicated, separately-constructed Solver rather than reusing the
    // shared `solver_` fixture - switching that shared instance's language
    // would leak into every other test class sharing it (see
    // app_tests_main.cpp). kDeferred + an explicit loadDictionaries() call
    // (mirroring main.cpp's own startup sequence) avoids wastefully loading
    // the full English dictionary first, since setLanguageCode() only
    // triggers a reload once dictionaries have already loaded once.
    countdown::app::Solver french_solver(nullptr,
                                          countdown::app::Solver::DictionaryLoad::kDeferred);
    french_solver.setLanguageCode(QStringLiteral("fr"));
    french_solver.loadDictionaries();

    QCOMPARE(french_solver.rackSize(), 10);

    const QString rack = french_solver.randomRack();
    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 10);
    QCOMPARE(rack, rack.toUpper());
}
