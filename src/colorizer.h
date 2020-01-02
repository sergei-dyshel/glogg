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

#include "color_scheme.h"
#include "range.h"

#include <iostream>
#include <list>
#include <vector>

#include <QDebug>
#include <QString>

class LogStream;

struct Token {
    Token() = default;
    Token(const Range &range_, const QString &colorScope_)
        : range(range_), colorScope(colorScope_)
    {}

    bool operator==(const Token& token) const {
      return range == token.range && colorScope == token.colorScope;
    }

    Range range;
    QString colorScope;
}; // struct Token

inline QDebug& operator<<(QDebug& debug, const Token& token) {
    QDEBUG_COMPAT(debug);
    return debug << token.range << ": " << token.colorScope;
}

void filterTokensByScheme(std::list<Token> &tokens,
                          const ColorScheme &scheme);

void mergeSyntaxTokens(std::list<Token> &upperTokens,
                       const std::list<Token> &syntaxTokens);

void addLowerToken(std::list<Token> &tokens, const Token& lower);
