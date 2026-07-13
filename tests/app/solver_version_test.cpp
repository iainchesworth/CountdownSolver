#include "solver_version_test.hpp"

#include <countdown/version.hpp>

#include <QString>
#include <QTest>

void VersionTest::versionDetails_nonEmpty() {
    const QString details = solver_.versionDetails();

    QVERIFY(!details.isEmpty());
    QVERIFY(details.contains(QStringLiteral("CountdownSolver")));
}

void VersionTest::shortVersion_matchesLibraryVersion() {
    const QString expected = QString::fromUtf8(countdown::version_string.data(),
                                                 static_cast<qsizetype>(countdown::version_string.size()));
    QCOMPARE(solver_.shortVersion(), expected);
}
