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

#include "config_node.h"
#include "color_scheme.h"
#include "syntax.h"

class StructConfig {
public:
    StructConfig() = default;

    void Load();
    const ColorScheme &colorScheme() const { return colorScheme_; }
    const SyntaxCollection &syntaxColl() const { return syntaxColl_; }
    bool checkForIssues() const;

    static const StructConfig &instance();
    static void loadDefault();
    static void reload();

private:
    void load();

    ColorScheme colorScheme_;
    SyntaxCollection syntaxColl_;
};