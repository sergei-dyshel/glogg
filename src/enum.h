/*
 * Copyright (C) 2018-2019 Sergei Dyshel and other contributors
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
