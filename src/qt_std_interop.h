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