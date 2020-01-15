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

#include "range.h"
#include "fwd.h"
#include "log.h"
#include "config_node.h"
#include "qt_std_interop.h"
#include "struct_stream.h"

#include <iostream>

#include <QColor>
#include <QString>
#include <QPalette>

#include <unordered_map>
#include <utility>

using namespace std::rel_ops;

struct TextColor final {
    TextColor(const QColor &fore = QColor(), const QColor &back = QColor())
        : foreground(fore), background(back)
    {}

    TextColor(const char *foreName);

    TextColor(const std::initializer_list<QColor> &init_list);

    explicit TextColor(const ConfigNode& node);

    QColor foreground;
    QColor background;

private:
    static QColor colorFromNode(const ConfigNode& node);
}; // struct TextColor

class ColorScheme final {
  private:
    QPalette palette;

  public:
    explicit ColorScheme(const QString &name = "",
                         const Location &location = Location());

    using User = std::unordered_map<QString, QColor>;
    using Map = std::map<QString, ColorScheme>;

    // Used for tests
    ColorScheme &setText(const TextColor &text);
    ColorScheme &setSelection(const TextColor &selection);
    ColorScheme &setQuickFind(const TextColor &quickFind);
    ColorScheme &addUser(const User &user, bool override_ = true);

    static Map loadAll(const ConfigNode &node);

    const QString &name() const { return name_; }

    TextColor scopeColor(const QString& name) const;
    TextColor scopeColor(const QString &name,
                         const TextColor &defaultColor) const;
    bool hasScope(const QString &name) const;

    const User &userScopes() const { return user_; }

    TextColor text;
    TextColor selection;
    TextColor quickFind;
    TextColor lineNumbers;

    struct {
        QColor background;
        QColor normal;
        QColor match;
        QColor mark;
    } bullets;

    static const QString TEXT;
    static const QString SELECTION;
    static const QString QUICK_FIND;
    static const QString HIGHLIGHT;

    static const unsigned HIGHLIGHT_COUNT;

    QString highlightScope(unsigned i) const;

    bool operator==(const ColorScheme &other) const;

    TextColor highlightColor(unsigned i) const;

  private:
    using Defs = std::map<QString, QColor>;

    void addDefs(const ConfigNode& node, Defs &defs);
    void addScopes(const ConfigNode& node, const Defs &defs);

    static QColor readColor(const ConfigNode &node, const Defs *defs = nullptr,
                     const User *userScopes = nullptr);
    TextColor readTextColor(const ConfigNode& node, const Defs &defs);

    TextColor withTextBackground(const QColor &foreground) const;

    void generateColorsMap();

    std::vector<TextColor> highlight_;

    QString name_;
    Location location_;
    User user_;
    std::unordered_map<QString, TextColor> colors_;
};

bool operator==(const TextColor &color1, const TextColor &color2);
StructStream& operator<<(StructStream &ss, const ColorScheme &scheme);
StructStream& operator<<(StructStream &ss, const QColor &color);
StructStream& operator<<(StructStream &ss, const TextColor &color);

DEFINE_QDEBUG_SHIFT_WITH_STRUCT(TextColor)
DEFINE_QDEBUG_SHIFT_WITH_STRUCT(ColorScheme)
DEFINE_STREAM_SHIFT_WITH_QDEBUG(ColorScheme)
