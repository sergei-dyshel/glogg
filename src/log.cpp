#include "log.h"

#include "utils.h"

#include <stdio.h>

#include <QFileInfo>
#include <QTextStream>

const char *LogContext::levelNames_[] = {"", "ERROR", "WARNING", "INFO", "DEBUG",
                                       "TRACE"};
const char *LogStream::QT_PATTERN = "- %{time hh:mm:ss.zzz} %{category} "
                              "%{file}:%{line}: [%{function}] %{message}";

LogContext::LogContext(const char *file, unsigned line, const char *func,
                       TLogLevel level)
    : level(level), file_(fileName(file)), line_(line), func_(func),
      qtContext_(file_.c_str(), line_, func_.c_str(), levelNames_[level])
{}

LogContext::LogContext(const LogContext &other)
    : level(other.level), file_(other.file_), line_(other.line_),
      func_(other.func_),
      qtContext_(file_.c_str(), line_, func_.c_str(), levelNames_[level])
{}

std::string LogContext::fileName(const char *file)
{
    return QFileInfo(file).fileName().toStdString();
}

LogStream::LogStream(const LogContext &context)
    : d(&str_), context_(context), str_("")
{}

LogStream &LogStream::compat()
{
    d.noquote();
    d.nospace();
    return *this;
}

bool LogStream::isPatternSet_ = false;

QString LogStream::fullMessage() const
{
    if (!isPatternSet_) {
        qSetMessagePattern(QT_PATTERN);
        isPatternSet_ = true;
    }
    return qFormatLogMessage(QtDebugMsg, context_.qtContext(), str_);
}

Log::Log(const LogContext &context, bool oldCompat) : LogStream(context)
{
    if (oldCompat)
        compat();
}

TLogLevel Log::level_;
QFile Log::file_;
std::unique_ptr<QTextStream> Log::stream_;
QMutex Log::mutex_;
bool Log::isConfigured_ = false;

void Log::configure(TLogLevel level, const QString &fileName)
{
    level_ = level;
    if (!fileName.isNull()) {
        file_.setFileName(fileName);
        file_.open(QIODevice::WriteOnly);
    } else {
        file_.open(stderr, QIODevice::WriteOnly);
    }
}

Log::~Log()
{
    QMutexLocker locker(&mutex_);
    if (!isConfigured_) {
        configure(logTRACE);
        isConfigured_ = true;
    }
    if (context_.level > level_)
        return;
    QTextStream stream(&file_);
    stream << fullMessage() << '\n';
    stream.flush();
    file_.flush();
}

QString ellipsize(const QString &str, int len)
{
    static const QString separator("...");
    if (str.length() <= len)
        return str;
    int visibleLen = str.length() - separator.length();
    int leftLen = visibleLen / 2;
    int rightLen = visibleLen - leftLen;
    return str.left(leftLen) + separator + str.right(rightLen);
}
