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

#ifndef __LOG_H__
#define __LOG_H__

#include <cstdio>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <set>
#include <stack>

#include <QDebug>
#include <QFile>
#include <QDebugStateSaver>
#include <QMessageLogContext>
#include <QMutex>

enum TLogLevel {logNoLevel, logERROR, logWARNING, logINFO, logDEBUG, logTRACE};

#define LOG(level) (Log(LOG_HERE(level), true))

#define HERE (LogContext(__FILE__, __LINE__, __PRETTY_FUNCTION__))
#define LOG_HERE(level)                                                        \
    LogContext(__FILE__, __LINE__, __PRETTY_FUNCTION__, level)

#define NEW_LOG(level) (Log(LOG_HERE(level)))

#ifdef ENABLE_TRACE
#define TRACE NEW_LOG(logTRACE)
#else
#define TRACE NoLog()
#endif

#define DEBUG NEW_LOG(logDEBUG)
#define INFO NEW_LOG(logINFO)
#define WARN NEW_LOG(logWARNING)
#define ERROR NEW_LOG(logERROR)

#define LOG_EXPR_AS(name, expr)                                                \
    QD_PUSH_STATE << NOSPACE << name "=" << (expr) << QD_POP_STATE

#define LOG_EXPR(expr) LOG_EXPR_AS(#expr, expr)

#define LOG_MEMBER(this, member) LOG_EXPR_AS(#member, (this)->member)

#define LOG_WRAP(open, expr, close)                                            \
    QD_PUSH_STATE << NOSPACE << open << (expr) << close << QD_POP_STATE

#define LOG_IN_BRACKETS(expr) LOG_WRAP("[", expr, "]")

#define DEBUG_THIS DEBUG << LOG_IN_BRACKETS(static_cast<void*>(this))
#define INFO_THIS DEBUG << LOG_IN_BRACKETS(static_cast<void*>(this))

#define QDEBUG_DEFINE_OPERATOR(type, expr)                                     \
    inline QDebug &operator<<(QDebug &debug, const type &val)                  \
    {                                                                          \
        return debug << expr;                                                  \
    }

#define QDEBUG_DEFINE_OPERATOR_STREAMED(type)                                  \
    inline QDebug &operator<<(QDebug &debug, const type &val)                  \
    {                                                                          \
        std::stringstream ss;                                                  \
        ss << val;                                                             \
        return debug << QString::fromStdString(ss.str());                      \
    }

#define QDEBUG_DEFINE_OPERATOR_LIST_CONTAINER(type)                            \
    template <typename T> QDebug &operator<<(QDebug &debug, type<T> list)      \
    {                                                                          \
        QDEBUG_COMPAT(debug);                                                  \
        debug.quote();                                                         \
        debug << "[";                                                          \
        unsigned numElems = 0;                                                 \
        for (const auto &elem : list) {                                        \
            debug << elem;                                                     \
            if (++numElems < list.size())                                      \
                debug << ", ";                                                 \
        }                                                                      \
        return debug << "]";                                                   \
    }

#define QDEBUG_DEFINE_OPERATOR_MAP_CONTAINER(type)                             \
    template <typename K, typename V>                                          \
    QDebug &operator<<(QDebug &debug, type<K, V> map)                          \
    {                                                                          \
        QDEBUG_COMPAT(debug);                                                  \
        debug.quote();                                                         \
        debug << "{";                                                          \
        unsigned numElems = 0;                                                 \
        for (const auto &elem : map) {                                         \
            debug << elem.first << ": " << elem.second;                        \
            if (++numElems < map.size())                                       \
                debug << ", ";                                                 \
        }                                                                      \
        return debug << "}";                                                   \
    }

class LogContext final {
  public:
    LogContext(const char *file, unsigned line, const char *func,
               TLogLevel level = logNoLevel);
    const QMessageLogContext &qtContext() const { return qtContext_; }
    LogContext(const LogContext &other);

    TLogLevel level;
  private:
    static std::string fileName(const char *file);
    static const char *levelNames_[];

    std::string file_;
    unsigned line_;
    std::string func_;

    QMessageLogContext qtContext_;
};

#define QDEBUG_COMPAT(debug)                                                   \
    QDebugStateSaver save(debug);                                              \
    debug.nospace().noquote()

enum QDebugManip {
    NOQUOTE,
    NOSPACE,
    QUOTE,
    SPACE,
};

enum LogStreamManip {
    QD_PUSH_STATE,
    QD_POP_STATE
};

class LogStream {
  public:

    LogStream(const LogStream &);
    LogStream &operator=(const LogStream &) = delete;

    LogStream(const LogContext &context);
    LogStream &compat();
    QString fullMessage() const;

    const QString &message() const { return str_; }

    LogStream &operator<<(LogStreamManip manip);
    template <typename T>
    friend LogStream &operator<<(LogStream &, const T &val);

    QDebug &qdebug() { return d; }

  protected:
    LogContext context_;

  private:
    static bool isPatternSet_;
    static const char *QT_PATTERN;

    std::stack<std::shared_ptr<QDebugStateSaver>> stateStack;

    QString str_;
    QDebug d;
};

class Log final : public LogStream {
  public:
    Log(const LogContext &context, bool oldCompat = false);
    ~Log();

    static void configure(TLogLevel level, const QString &fileName = QString());

    template <typename T> Log &operator<<(const T &val);

  private:
    static TLogLevel level_;
    static QFile *file_;
    static std::unique_ptr<QTextStream> stream_;
    static QMutex mutex_;
    static bool isConfigured_;
};

struct NoLog final {
    template <typename T> const NoLog &operator<<(const T &) const
    {
        return *this;
    }
};

QString ellipsize(const QString &str, int len = 30);

// Implementation

template <typename T> LogStream &operator<<(LogStream &stream, const T &val)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#endif
#pragma GCC diagnostic ignored "-Wnonnull-compare"
    stream.d << val;
#pragma GCC diagnostic pop
    return stream;
}

template <typename T> Log &Log::operator<<(const T &val)
{
    static_cast<LogStream &>(*this) << val;
    return *this;
}

QDebug &operator<<(QDebug &, LogStreamManip) = delete;

QDEBUG_DEFINE_OPERATOR(std::string, QString::fromStdString(val))

template <typename T>
QDEBUG_DEFINE_OPERATOR_STREAMED(std::shared_ptr<T>)

QDEBUG_DEFINE_OPERATOR_LIST_CONTAINER(std::list)
QDEBUG_DEFINE_OPERATOR_LIST_CONTAINER(std::set)
QDEBUG_DEFINE_OPERATOR_MAP_CONTAINER(std::unordered_map)

QDebug &operator<<(QDebug &debug, QDebugManip manip);

#endif //__LOG_H__
