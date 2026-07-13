#pragma once

// QtGlobal (rather than QObject below) so Q_OS_LINUX is defined before the
// #ifdef is evaluated regardless of what, if anything, a including .cpp
// already pulled in.
#include <QtGlobal>

// Linux-only: countdown::app::logging is only built/available on Linux (see
// the Q_OS_LINUX guard around its include in the .cpp), so this entire class
// only exists on that platform. app_tests_main.cpp guards its use of
// LoggingTest the same way.
#ifdef Q_OS_LINUX

#include <QObject>

// countdown::app::logging::install(): console-only fallback when there's no
// config dir, and the log-file rotation contract.
class LoggingTest : public QObject {
    Q_OBJECT

private slots:
    void logging_installWithoutConfigDirSkipsFileSink();
    void logging_installRotatesAndWritesAllSeverities();
};

#endif  // Q_OS_LINUX
