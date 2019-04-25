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

#include "colorizer.h"
#include "log.h"
#include "qt_std_interop.h"

void mergeSyntaxTokens(std::list<Token> &upperTokens,
                       const std::list<Token> &syntaxTokens)
{
    for (auto iter = syntaxTokens.crbegin(); iter != syntaxTokens.crend();
         ++iter)
        addLowerToken(upperTokens, *iter);
}

void addLowerToken(std::list<Token> &upperTokens, const Token& lower)
{
    unsigned lower_start = lower.range.start;

    auto upper  = upperTokens.begin();

    for (; upper != upperTokens.end(); ++upper) {
        if (upper == upperTokens.end())
            break;

        if (upper->range.start <= lower_start) {
            if (upper->range.end >= lower.range.end)
                return;
            lower_start = qMax(upper->range.end, lower_start);
            continue;
        }

        if (upper->range.start >= lower.range.end)
          break;

        upperTokens.insert(
            upper,
            Token(Range(lower_start, upper->range.start), lower.colorScope));
        lower_start = upper->range.end;
    }

    if (lower_start < lower.range.end) {
        upperTokens.insert(
            upper, Token(Range(lower_start, lower.range.end), lower.colorScope));
    }
}

void filterTokensByScheme(std::list<Token> &tokens,
                          const ColorScheme &scheme)
{
    auto iter = tokens.begin();
    while (iter != tokens.end()) {
        if (!scheme.hasScope(iter->colorScope))
            tokens.erase(iter++);
        else
            ++iter;
    }
}
