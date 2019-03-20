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

#include "yaml-cpp/yaml.h"
#include "log.h"
#include "exception.h"

#include <QString>
#include <QDebug>

#include <unordered_map>

#define DEFINE_QDEBUG_SHIFT_WITH_STRUCT(type)                                  \
    inline QDebug &operator<<(QDebug &debug, const type &val)                  \
    {                                                                          \
        QDEBUG_COMPAT(debug);                                                  \
        StructStream ss;                                                       \
        ss << val;                                                             \
        return debug << ss.toString();                                         \
    }

#define DEFINE_STRUCT_SHIFT_FOR_MAP_TYPE(Type)                                 \
    template <typename K, typename V>                                          \
    inline StructStream &operator<<(StructStream &ss, const Type<K, V> &map)   \
    {                                                                          \
        ss << BEGIN_MAP;                                                       \
        for (const auto &kv : map)                                             \
            ss << kv.first << kv.second;                                       \
        return ss << END_MAP;                                                  \
    }

#define DEFINE_STRUCT_SHIFT_FOR_SEQ_TYPE(Type)                                 \
    template <typename K>                                                      \
    inline StructStream &operator<<(StructStream &ss, const Type<K> &seq)      \
    {                                                                          \
        ss << BEGIN_SEQ;                                                       \
        for (const auto &item : seq)                                           \
            ss << item;                                                        \
        return ss << END_SEQ;                                                  \
    }

static constexpr auto BEGIN_MAP = YAML::BeginMap;
static constexpr auto END_MAP = YAML::EndMap;
static constexpr auto BEGIN_SEQ = YAML::BeginSeq;
static constexpr auto END_SEQ = YAML::EndSeq;

class StructStream final {
  public:
    StructStream();
    QString toString() const;


    template <typename T> StructStream& operator<<(const T& val)
    {
        emitter_ << val;
        if (!emitter_.good())
            throw Exception(HERE) << "Error writing to structured stream:"
                                  << emitter_.GetLastError();
        return *this;
    }

  private:
    YAML::Emitter emitter_;
};

YAML::Emitter &operator<<(YAML::Emitter &emitter, const QString &str);

template <typename T1, typename T2>
StructStream &operator<<(StructStream &ss, const std::pair<T1, T2> &pair)
{
    return ss << BEGIN_SEQ << pair.first << pair.second << END_SEQ;
}

DEFINE_STRUCT_SHIFT_FOR_MAP_TYPE(std::unordered_map)
DEFINE_STRUCT_SHIFT_FOR_MAP_TYPE(std::map)

DEFINE_STRUCT_SHIFT_FOR_SEQ_TYPE(std::list)
DEFINE_STRUCT_SHIFT_FOR_SEQ_TYPE(std::vector)
