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
#include "colorizer.h"
#include "config_node.h"
#include "exception.h"
#include "qt_std_interop.h"
#include "enum.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QRegularExpression>
#include <QString>

struct SyntaxParsingState;
class Syntax;
class SyntaxCollection;

class SyntaxRule final {
public:
    enum class SearchType;

    SyntaxRule(const QString parent, const ConfigNode &node);

    void apply(const QString &line, SyntaxParsingState& state) const;

    ConfigError error(const LogContext &context) const;

    friend class Syntax;

    QString fullName() const {
        return parentName_ + "/" + location_.toShortString();
    }

    enum class SearchType {
        MATCH,
        ALL,
    };

    static const QString GROUP_LINE;
    static const QString GROUP_MATCH;
    static const QString GROUP_GROUP;

  private:
    void PostInit();
    void processMatch(const QRegularExpressionMatch &match,
                      SyntaxParsingState &state) const;

    QString parentName_;
    Location location_;
    QString matchGroup_;
    QRegularExpression regExp_;
    SearchType searchType_;

    std::unordered_set<QString> regExpGroups_;

    std::unordered_map<QString, QString> colorize_;
};

class Syntax final {
  public:
    Syntax();
    Syntax(const QString &name, const ConfigNode &node, bool runTests);

    std::list<Token> parse(const QString &line) const;

    std::unordered_set<QString> usedScopes() const { return usedScopes_; }

    const QString &name() const { return name_; }

    QString nameAndLocation() const
    {
        return name_ + "/" + location_.toString();
    }

    ConfigError error(const LogContext &ctx) const;

    friend class SyntaxCollection;

  private:
    Syntax &addRule(const SyntaxRule &rule);
    bool runTest(const QString &line, const ConfigNode &node);

    QString name_;
    Location location_;
    std::unordered_set<QString> usedScopes_;
    std::unordered_set<QString> usedGroups_;
    std::unordered_set<QString> usedNames_;
    std::vector<SyntaxRule> rules_;
};

class SyntaxCollection final {
public:
    SyntaxCollection() = default;
    SyntaxCollection(const ConfigNode &node, bool runTests);

    void merge(const SyntaxCollection &other);

    std::list<Token> parse(const QString &line) const;

    const std::list<Syntax> syntaxes() const { return syntaxes_; }

  private:
    std::list<Syntax> syntaxes_;
};

QDEBUG_DEFINE_ENUM(SyntaxRule::SearchType)

QDebug &operator<<(QDebug &d, const SyntaxRule &rule);