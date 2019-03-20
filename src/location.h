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

class Location final {
  public:
    Location() = default;
    Location(const QString &path, unsigned lineNumber);
    const QString &path() const { return path_; }
    QString fileName() const;
    unsigned lineNumber() const { return lineNumber_; }

    QString toString(bool fileNameOnly = false) const;
    QString toShortString() const;
    operator QString() const;

  private:
    QString path_;
    unsigned lineNumber_ = 0;
};

QDebug& operator<<(QDebug &debug, const Location &location);