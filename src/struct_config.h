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

struct StructConfigFiles {
    QStringList colorsFiles;
    QStringList syntaxFiles;
};

class StructConfig {
  public:
    using ColorSchemeMap = std::map<QString, ColorScheme>;

    StructConfig() = default;
    StructConfig(const QStringList &colorsFiles, const QStringList &syntaxFiles,
                 bool runTests, bool stopOnError);
    StructConfig(const StructConfigFiles &configFiles, bool runTests,
                 bool stopOnError);

    StructConfig(const StructConfig &) = delete;

    const ColorSchemeMap &colorSchemes() const { return colorSchemes_; }
    const SyntaxCollection &syntaxColl() const { return syntaxColl_; }
    bool checkForIssues() const;

    static void scanDirs(const QStringList &dirs, StructConfigFiles &files);
    static void scanDirsAndFiles(const QStringList &dirsAndFiles,
                                 StructConfigFiles &files);

    static const QString COLORS_GLOB_PATTERN;
    static const QString SYNTAX_GLOB_PATTERN;

  private:
    ColorSchemeMap colorSchemes_;
    SyntaxCollection syntaxColl_;
};
