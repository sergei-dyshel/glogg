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

#include "highlights.h"
#include "struct_config_store.h"

Highlights::Highlights()
    : patterns_(ColorScheme::HIGHLIGHT_COUNT),
      regexes_(ColorScheme::HIGHLIGHT_COUNT)
{}

void Highlights::generateRegex(unsigned colorIndex)
{
    regexes_[colorIndex]
        = QRegularExpression(stringList2Regex(patterns_.at(colorIndex)));
}

void Highlights::addPattern(const QString &pattern, unsigned colorIndex)
{
    removePattern(pattern);
    patterns_.at(colorIndex).append(pattern);
    generateRegex(colorIndex);
    DEBUG << "Added pattern" << pattern;
}

void Highlights::removePattern(const QString &pattern)
{
    for (unsigned i = 0; i < patterns_.size(); ++i)
        if (patterns_.at(i).removeAll(pattern)) {
            DEBUG << "Removed pattern" << pattern;
            generateRegex(i);
        }
}

bool Highlights::hasPattern(const QString &pattern) const
{
    for (const auto &colorPatterns : patterns_)
        if (colorPatterns.contains(pattern))
            return true;
    return false;
}

std::list<Token> Highlights::colorize(const QString &line) const
{
    std::list<Token> tokens;
    const auto &colorScheme = StructConfigStore::get().colorScheme();
    for (unsigned i = 0; i < patterns_.size(); ++i) {
        if (patterns_.at(i).isEmpty())
            continue;
        auto iter = regexes_.at(i).globalMatch(line);
        while (iter.hasNext()) {
            auto match = iter.next();
            addLowerToken(
                tokens, Token(Range(match.capturedStart(), match.capturedEnd()),
                              colorScheme.highlightScope(i)));
        }
    }
    return tokens;
}

bool Highlights::empty() const
{
    for (const auto &colorPatterns : patterns_)
        if (!colorPatterns.empty())
            return false;
    return true;
}

const QStringList &Highlights::getPatterns(unsigned colorIndex) const
{
    return patterns_.at(colorIndex);
}


void Highlights::clear()
{
    for (auto &colorPatterns : patterns_)
        colorPatterns.clear();
    // no need to generate regex since it's not used until some pattern is added
    // again
}

std::multimap<unsigned, QString> Highlights::getAllPatterns() const
{
    std::multimap<unsigned, QString> result;
    for (unsigned i = 0; i < patterns_.size(); ++i)
        for (const auto &pattern : patterns_.at(i)) {
            DEBUG << "TEMP:" << pattern;
            result.emplace(i, pattern);
        }
    return result;
}

