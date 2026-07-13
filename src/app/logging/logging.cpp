#include "logging/logging.hpp"

#include "platform/platform.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QTextStream>

#include <cstdio>
#include <memory>

Q_LOGGING_CATEGORY(lcApp, "countdown.app")
Q_LOGGING_CATEGORY(lcQml, "countdown.qml")
Q_LOGGING_CATEGORY(lcSolver, "countdown.solver")
Q_LOGGING_CATEGORY(lcDictionary, "countdown.dictionary")

namespace countdown::app::logging {
namespace {

// A long-running session shouldn't grow the log file without bound; once it
// crosses this size at startup, the previous run's file is kept as a single
// ".1" backup and a fresh file is started.
constexpr qint64 kMaxLogFileSize = 1'000'000;  // ~1 MB

// Owned for the lifetime of the process: qInstallMessageHandler's callback
// can run until the process exits (including during static destruction of
// other globals), so this is intentionally never deleted rather than risk a
// use-after-free from destruction order.
QFile* g_log_file = nullptr;
QMutex* g_log_mutex = new QMutex();

[[nodiscard]] char level_letter(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:    return 'D';
        case QtInfoMsg:     return 'I';
        case QtWarningMsg:  return 'W';
        case QtCriticalMsg: return 'C';
        case QtFatalMsg:    return 'F';
    }
    return '?';
}

[[nodiscard]] QString format_line(QtMsgType type, const QMessageLogContext& context,
                                   const QString& message) {
    QString line;
    QTextStream stream(&line);
    stream << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << " [" << level_letter(type)
           << "] " << (context.category ? context.category : "default") << ": " << message;
    return line;
}

void handle_message(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    const QString line = format_line(type, context, message);

    QMutexLocker lock(g_log_mutex);

    std::FILE* console = (type == QtDebugMsg || type == QtInfoMsg) ? stdout : stderr;
    std::fprintf(console, "%s\n", qPrintable(line));

    if (g_log_file != nullptr) {
        QTextStream file_stream(g_log_file);
        file_stream << line << '\n';
        g_log_file->flush();
    }

    if (type == QtFatalMsg) {
        std::fflush(nullptr);
        std::abort();
    }
}

// Keeps at most one backup: <path> -> <path>.1, dropping any older backup.
void rotate_if_oversized(const QString& path) {
    if (const QFileInfo info(path); !info.exists() || info.size() < kMaxLogFileSize) {
        return;
    }
    const QString backup = path + QStringLiteral(".1");
    QFile::remove(backup);
    QFile::rename(path, backup);
}

}  // namespace

void install() {
    const platform::PlatformInfo info = platform::current();
    if (!info.config_dir.empty()) {
        const QString log_dir = QString::fromStdString((info.config_dir / "logs").string());
        if (QDir().mkpath(log_dir)) {
            const QString log_path = log_dir + QStringLiteral("/countdown-solver.log");
            rotate_if_oversized(log_path);

            auto file = std::make_unique<QFile>(log_path);
            if (file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                g_log_file = file.release();
            }
        }
    }

    qInstallMessageHandler(handle_message);
}

}  // namespace countdown::app::logging
