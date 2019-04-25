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

#include "fwd.h"
#include "syntax.h"

#include <vector>
#include <map>

class Highlights final {
  public:
    Highlights();
    void addPattern(const QString &pattern, unsigned colorIndex);
    bool hasPattern(const QString &pattern) const;
    void removePattern(const QString &pattern);
    void clear();
    bool empty() const;
    const QStringList &getPatterns(unsigned colorIndex) const;
    std::multimap<unsigned, QString> getAllPatterns() const;

    std::list<Token> colorize(const QString &line) const;

  private:
    void generateRegex(unsigned colorIndex);

    std::vector<QStringList> patterns_;
    std::vector<QRegularExpression> regexes_;
};