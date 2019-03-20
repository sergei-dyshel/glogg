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

#include "struct_config.h"
#include "fwd.h"

#include <QSettings>
#include <memory>

class StructConfigStore {
  public:
    StructConfigStore();

    static StructConfigStore &get();
    static void init();

    const SyntaxCollection &syntaxColl() { return config_.syntaxColl(); }
    const QString &colorSchemeName() const { return colorSchemeName_; }
    const ColorScheme &colorScheme() const;
    void reload();

    void setColorScheme(const QString &name);

    QStringList colorSchemeNames() const;

    void saveSettings();

  private:
    Settings &settings_;
    StructConfig config_;
    QString colorSchemeName_;
    ColorScheme defaultColorScheme_;
};