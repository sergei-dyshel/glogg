#pragma once

#include "exception.h"
#include "qt_std_interop.h"

#include <unordered_map>

#include <QString>
#include <QStringList>

#define QDEBUG_DEFINE_ENUM(E)                                                  \
    static inline QDebug& operator<<(QDebug& debug, E e)                       \
    {                                                                          \
        QDEBUG_COMPAT(debug);                                                  \
        return debug << Enum<E>::name << "::" << Enum<E>::toString(e);         \
    }

template <typename E> struct Enum {
    static const QString name;
    static const std::unordered_map<E, QString> strings;

    static QStringList stringList() {
        return makeStringListFromValues(strings);
    }

    static QString toString(E e)
    {
        if (!strings.count(e))
            throw Exception(HERE) << "Invalid value for enum" << name << ":"
                                  << static_cast<int>(e);
        return strings.at(e);
    }

    static E fromString(const QString &str)
    {
        for (const auto &kv : strings)
            if (str == kv.second)
                return kv.first;
        throw Exception(HERE) << "Cannot convert" << str << "to enum " << name;
    }
};

// TODO: move to unittests
// Example

enum class SampleEnum {
    ONE,
    TWO
};

QDEBUG_DEFINE_ENUM(SampleEnum)