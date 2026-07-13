#pragma once

#include <QLoggingCategory>

// One category per subsystem so output can be filtered per area at runtime
// via QT_LOGGING_RULES / qtlogging.ini (e.g. "countdown.dictionary.debug=true")
// without a rebuild.
Q_DECLARE_LOGGING_CATEGORY(lcApp)         // application lifecycle (main.cpp)
Q_DECLARE_LOGGING_CATEGORY(lcQml)         // QML engine warnings/errors
Q_DECLARE_LOGGING_CATEGORY(lcSolver)      // Solver facade (solveNumbers/solveLetters/solveConundrum)
Q_DECLARE_LOGGING_CATEGORY(lcDictionary)  // dictionary loading

namespace countdown::app::logging {

// Installs the process-wide Qt message handler: every qCDebug/qCWarning/...
// call is written to stdout/stderr and appended to a rotating log file under
// the platform config dir (see platform::current().config_dir). Safe to call
// even when no config dir is available - the file sink is then skipped and
// only the console sink is used. Must be called once, before anything that
// might log (ideally the first line of main()).
void install();

}  // namespace countdown::app::logging
