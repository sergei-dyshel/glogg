/*
 * Copyright (C) 2009, 2010 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "log.h"
#include "exception.h"

#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <QFileInfo>
#include <QTextStream>

const char *LogContext::levelNames_[] = {"", "ERROR", "WARNING", "INFO", "DEBUG",
                                       "TRACE"};

static const char *COLOR_RESET = "\x1B[0m";
static const char *COLOR_RED = "\x1B[31m";
static const char *COLOR_YELLOW = "\x1B[33m";
static const char *COLOR_WHITE = "\x1B[37m";

const char *levelColors[]
    = {nullptr, COLOR_RED, COLOR_YELLOW, COLOR_WHITE, nullptr, nullptr};

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
    : context_(context), str_(""), d(&str_)
{}

LogStream &LogStream::compat()
{
    d.noquote();
    d.nospace();
    return *this;
}

LogStream::LogStream(const LogStream &other)
    : context_(other.context_), stateStack(), str_(other.str_), d(other.d)
{}

LogStream &LogStream::operator<<(LogStreamManip manip)
{
    switch (manip) {
        case QD_PUSH_STATE:
            stateStack.push(std::make_shared<QDebugStateSaver>(d));
            break;
        case QD_POP_STATE:
            stateStack.pop();
            break;
    }
    return *this;
}

QString LogStream::fullMessage() const
{
    return qFormatLogMessage(QtDebugMsg, context_.qtContext(), str_);
}

Log::Log(const LogContext &context, bool oldCompat) : LogStream(context)
{
    if (oldCompat)
        compat();
}

const char *Log::QT_PATTERN = "- %{time hh:mm:ss.zzz} %{category} "
                              "%{file}:%{line}: [%{function}] %{message}";
TLogLevel Log::level_;
std::unique_ptr<QTextStream> Log::stream_;
QMutex Log::mutex_;
QFile *Log::file_;

// break on this function to get stacktrace
void Log::qtLogHandler(QtMsgType type, const QMessageLogContext &context,
                       const QString &msg)
{
    file_->write(qFormatLogMessage(type, context, msg).toLocal8Bit());
    file_->flush();
#ifndef NDEBUG
    throw ASSERT_HERE << "Asserting on QT message:" << msg;
#endif
}

void Log::configure(TLogLevel level, const QString &fileName)
{
    level_ = level;
    if (!fileName.isNull()) {
        file_ = new QFile(fileName);
        file_->open(QIODevice::WriteOnly);
    } else {
        file_ = new QFile();
        file_->open(stderr, QIODevice::WriteOnly);
    }
    qSetMessagePattern(QT_PATTERN);
    qInstallMessageHandler(qtLogHandler);
}

QString Log::coloredMessage() const
{
    if (isatty(file_->handle())) {
        auto color = levelColors[context_.level];
        if (color) {
            return color + fullMessage() + COLOR_RESET;
        }
    }
    return fullMessage();
}

Log::~Log()
{
    QMutexLocker locker(&mutex_);
    if (context_.level > level_)
        return;
    // TODO: create stream one time, together with file_
    QTextStream stream(file_);
    stream << coloredMessage() << '\n';
    stream.flush();
    file_->flush();
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

QDebug &operator<<(QDebug &debug, QDebugManip manip)
{
    switch (manip) {
        case NOQUOTE:
            return debug.noquote();
        case QUOTE:
            return debug.quote();
        case NOSPACE:
            return debug.nospace();
        case SPACE:
            return debug.space();
    }
    return debug;
}

// static constructor
static bool _initialized = [] {
    Log::configure(logWARNING);
    return _initialized;
}();
