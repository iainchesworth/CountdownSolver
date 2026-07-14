#include "solver_logging_test.hpp"

#ifdef Q_OS_LINUX

#include "logging/logging.hpp"
#include "platform/platform.hpp"

#include <QFile>
#include <QIODevice>
#include <QTemporaryDir>
#include <QTest>

void LoggingTest::logging_installWithoutConfigDirSkipsFileSink() {
    const QByteArray previousXdg = qgetenv("XDG_CONFIG_HOME");
    const QByteArray previousHome = qgetenv("HOME");
    qunsetenv("XDG_CONFIG_HOME");
    qunsetenv("HOME");
    QVERIFY(countdown::platform::current().config_dir.empty());

    // install() must tolerate having nowhere to put a log file - console
    // output alone is still useful, and this must not crash.
    const QtMessageHandler previousHandler = qInstallMessageHandler(nullptr);
    countdown::app::logging::install();
    qCWarning(lcApp) << "console-only message";
    qInstallMessageHandler(previousHandler);

    if (previousXdg.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previousXdg);
    }
    if (previousHome.isNull()) {
        qunsetenv("HOME");
    } else {
        qputenv("HOME", previousHome);
    }
}

void LoggingTest::logging_installRotatesAndWritesAllSeverities() {
    QTemporaryDir configDir;
    QVERIFY(configDir.isValid());

    const QByteArray previousXdg = qgetenv("XDG_CONFIG_HOME");
    qputenv("XDG_CONFIG_HOME", configDir.path().toUtf8());

    const QString logPath =
        configDir.path() + QStringLiteral("/countdown-solver/logs/countdown-solver.log");
    const QtMessageHandler previousHandler = qInstallMessageHandler(nullptr);

    // 1st install(): no log file exists yet - exercises rotate_if_oversized()'s
    // "doesn't exist" early-return branch, then creates the file fresh.
    countdown::app::logging::install();
    qCDebug(lcApp) << "debug message";
    qCInfo(lcQml) << "info message";
    qCWarning(lcSolver) << "warning message";
    qCCritical(lcDictionary) << "critical message";
    QVERIFY(QFile::exists(logPath));
    QVERIFY(!QFile::exists(logPath + QStringLiteral(".1")));

    // 2nd install(): the file now exists but is tiny - exercises the "exists
    // but under the rotation threshold" early-return branch.
    countdown::app::logging::install();
    QVERIFY(!QFile::exists(logPath + QStringLiteral(".1")));

    // Grow the file past the rotation threshold, then install() a 3rd time -
    // exercises the actual rotation (rename to a ".1" backup).
    {
        QFile log(logPath);
        QVERIFY(log.open(QIODevice::Append));
        log.write(QByteArray(1'100'000, 'x'));
    }
    countdown::app::logging::install();
    QVERIFY(QFile::exists(logPath + QStringLiteral(".1")));

    qCWarning(lcApp) << "post-rotation message";
    qInstallMessageHandler(previousHandler);

    QFile log(logPath);
    QVERIFY(log.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString contents = QString::fromUtf8(log.readAll());
    QVERIFY(contents.contains(QStringLiteral("post-rotation message")));

    if (previousXdg.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previousXdg);
    }
}

#endif  // Q_OS_LINUX
