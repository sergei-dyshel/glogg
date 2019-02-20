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

#include <iostream>

#include <QColor>
#include <QString>

#include <unordered_map>

struct TextColor final {
    // TODO: remove default after writing tests
    explicit TextColor(const QColor &fore = QColor(), const QColor &back = QColor())
        : foreground(fore), background(back)
    {}

    explicit TextColor(const ConfigNode& node);

    QColor foreground;
    QColor background;

private:
    static QColor colorFromNode(const ConfigNode& node);
}; // struct TextColor

class ColorScheme final {
  public:
    explicit ColorScheme(const ConfigNode& node = ConfigNode());

    TextColor scopeColor(const QString& name) const;
    TextColor scopeColor(const QString& name, const TextColor &defaultColor) const;
    bool hasScope(const QString &name) const;

    TextColor text;
    TextColor selection;
    TextColor quickFind;

    static const QString TEXT;
    static const QString SELECTION;
    static const QString QUICK_FIND;

  private:

    QColor readColor(const ConfigNode& node, bool allowUser);
    TextColor readTextColor(const ConfigNode& node);

    std::unordered_map<QString, QColor> defs_;
    std::unordered_map<QString, QColor> user_;
};

bool operator==(const TextColor &color1, const TextColor &color2);
// TODO: define using https://stackoverflow.com/questions/23388739/c-relational-operators-generator
bool operator!=(const TextColor &color1, const TextColor &color2);
QDebug& operator<<(QDebug &debug, const TextColor &color);