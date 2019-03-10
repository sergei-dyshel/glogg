#pragma once

#include <QString>
#include <QDebug>
#include <QStringList>

#define DEFINE_STREAM_SHIFT_WITH_QDEBUG(type)                                  \
    inline std::ostream &operator<<(std::ostream &os, const type &val)         \
    {                                                                          \
        QString str;                                                           \
        QDebug dbg(&str);                                                      \
        dbg << val;                                                            \
        return os << str;                                                      \
    }

namespace std
{
    template <> struct hash<QString> {
        size_t operator()(const QString& str) const { return qHash(str); }
    };
}

template <typename Cont>
QStringList makeStringListFromValues(const Cont &cont)
{
    QStringList result;
    for (const auto &kv : cont) {
        result.push_back(kv.second);
    }
    return result;
}

inline std::ostream &operator<<(std::ostream &os, const QString &str)
{
    return os << str.toStdString();
}