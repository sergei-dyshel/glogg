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

#include "syntax.h"
#include "log.h"

struct SyntaxParsingState {
    std::unordered_map<QString, Token> groups;
    std::list<Token> tokens;
};

const QString SyntaxRule::GROUP_LINE = "LINE";
const QString SyntaxRule::GROUP_MATCH = "MATCH";
const QString SyntaxRule::GROUP_GROUP = "GROUP";

ENUM_DEFINE(SyntaxRule::SearchType, "SearchType",
            ({{SyntaxRule::SearchType::MATCH, "match"},
              {SyntaxRule::SearchType::ALL, "all"}}));

QString stringList2Regex(const QStringList &strings)
{
    QStringList escaped;
    for (const auto &str : strings)
        escaped.push_back(QRegularExpression::escape(str));
    return escaped.join('|');
}

#define RULE_TRACE TRACE << *this << ":"

SyntaxRule::SyntaxRule(const QString parent, const ConfigNode &node)
    : parentName_(parent), location_(node.location())
{
    node.assertProperties({"group", "match", "matchAll", "colorize"});
    matchGroup_ = node.member("group", GROUP_LINE).asString();
    if (node.hasMember("match") && node.hasMember("matchAll"))
        throw error(HERE)
            << "Can not use both 'match' and 'matchAll' at the same time";
    ConfigNode regexNode;
    if (node.hasMember("match")) {
        regexNode = node.requiredMember("match");
        searchType_ = SearchType::MATCH;
    }
    else if (node.hasMember("matchAll")) {
        regexNode = node.requiredMember("matchAll");
        searchType_ = SearchType::ALL;
    } else {
        throw error(HERE)
            << "One of 'match' or 'matchAll' properties must be present";
    }
    QString regex = regexNode.isScalar()
                        ? regexNode.asString()
                        : stringList2Regex(regexNode.asStringList());
    regExp_ = QRegularExpression(regex);

    auto colorize = node.member("colorize");
    if (colorize) {
        if (colorize.isScalar()) {
            QString group;
            switch (searchType_) {
            case SearchType::MATCH:
                group = GROUP_GROUP;
                break;
            case SearchType::ALL:
                group = GROUP_MATCH;
                break;
            default:
                throw error(HERE) << "Unsupported searchType";
            }
            colorize_.emplace(group, colorize.asString());
        }
        else if (colorize.isObject()) {
            for (const auto &colorRule :
                 node.requiredMember("colorize").members())
                colorize_.emplace(colorRule.first, colorRule.second.asString());
        }
        else {
            throw error(HERE) << "'colorize' is not string or object";
        }
        RULE_TRACE << colorize_;
    }
    PostInit();
}

ConfigError SyntaxRule::error(const LogContext &context) const
{
    return ConfigError(location_, context);
}

void SyntaxRule::PostInit()
{
    if (!regExp_.isValid())
        throw error(HERE) << "not a valid regular expression: "
                      << regExp_.pattern();

    for (auto group : regExp_.namedCaptureGroups()) {
        if (group == "")
            continue;
        if (group == GROUP_MATCH && searchType_ == SearchType::MATCH)
            throw error(HERE)
                << "Can not use" << GROUP_MATCH << "capture group together with"
                << SearchType::MATCH;
        if (group == GROUP_GROUP && searchType_ == SearchType::ALL)
            throw error(HERE)
                << "Cannot use " << GROUP_GROUP << "capture group together with"
                << SearchType::ALL;
        regExpGroups_.insert(group);
    }
    if (searchType_ == SearchType::ALL)
        for (const auto &kv : colorize_) {
            auto group = kv.first;
            if (group != GROUP_MATCH && !regExpGroups_.count(group))
                throw error(HERE) << "when using" << SearchType::ALL
                                  << ", can colorize only captured groups";
        }
}

void SyntaxRule::apply(const QString &line, SyntaxParsingState& state) const
{
    RULE_TRACE << "Applying to state: groups =" << state.groups
               << ", tokens =" << state.tokens;
    auto it = state.groups.find(matchGroup_);
    if (it == state.groups.end())
        return;

    auto range = it->second.range;
    QStringRef groupStr(&line, range.start, range.length());

    RULE_TRACE << "Matching" << regExp_.pattern() << "on" << groupStr;

    switch (searchType_) {
        case SearchType::MATCH:
            processMatch(regExp_.match(groupStr), state);
            return;
        case SearchType::ALL:
            auto iter = regExp_.globalMatch(groupStr);
            while (iter.hasNext()) {
                processMatch(iter.next(), state);
            }
            return;
    }
    throw ASSERT_HERE << "Invalid search type" << searchType_;
}

void SyntaxRule::processMatch(const QRegularExpressionMatch &match,
                              SyntaxParsingState &state) const
{
    if (!match.hasMatch()) {
        return;
    }
    RULE_TRACE << "matched " << match.capturedRef(0);
    for (auto group : regExpGroups_) {
        auto captured = match.capturedRef(group);
        if (captured.isNull() || captured.isEmpty())
            continue;
        RULE_TRACE << "captured" << captured << "as" << group;
        if (searchType_ == SearchType::MATCH)
            state.groups[group] = Token(Range(captured), group);
    }

    for (const auto &kv : colorize_) {
        auto group = kv.first;
        if (searchType_ == SearchType::MATCH) {
            if (group == GROUP_GROUP)
                group = matchGroup_;
            if (state.groups.count(group))
                state.groups.at(group).colorScope = kv.second;
        }
        else if (searchType_ == SearchType::ALL) {
            QStringRef captured
                = group == GROUP_MATCH && !regExpGroups_.count(group)
                      ? match.capturedRef(0)
                      : match.capturedRef(group);
            if (captured.isNull() || captured.isEmpty())
                continue;
            state.tokens.push_back(Token(Range(captured), kv.second));
        }
    }
}

QDebug &operator<<(QDebug &d, const SyntaxRule &rule)
{
    QDEBUG_COMPAT(d);
    return d << "(" << rule.fullName() << ")";
}

Syntax::Syntax() { usedGroups_.insert(SyntaxRule::GROUP_LINE); }

ConfigError Syntax::error(const LogContext &ctx) const
{
    return ConfigError(location_, ctx);
}

Syntax::Syntax(const QString &name, const ConfigNode &node, bool runTests)
    : Syntax()
{
    name_ = name;
    location_ = node.location();

    DEBUG << "Loading syntax" << name_;
    node.assertProperties({"rules", "tests"});
    for (const auto &elem : node.requiredMember("rules").elements())
        addRule(SyntaxRule(name, elem));

    if (runTests && node.hasMember("tests")) {
        bool ok = false;
        for (const auto &test : node.requiredMember("tests").members())
            ok |= runTest(test.first, test.second);
        if (!ok)
            throw error(HERE) << "Some tests failed";
    }
    DEBUG << "Loaded " << rules_.size() << " rules";
}

Syntax &Syntax::addRule(const SyntaxRule &rule)
{
    if (!usedGroups_.count(rule.matchGroup_))
        throw rule.error(HERE) << "matches group" << rule.matchGroup_
                               << "which is not provided by previous rules";
    for (const auto &group : rule.regExpGroups_)
        if (group != SyntaxRule::GROUP_MATCH)
            usedGroups_.insert(group);
    for (const auto &kv : rule.colorize_) {
        auto group = kv.first;
        if (group != SyntaxRule::GROUP_GROUP && group != SyntaxRule::GROUP_MATCH
            && !usedGroups_.count(group))
            throw rule.error(HERE)
                << "colorizes group" << group
                << "that is not provided by this or previous rules";
        usedScopes_.insert(kv.second);
    }
    DEBUG << "Added rule" << rule;

    rules_.push_back(rule);
    return *this;
}

std::list<Token> Syntax::parse(const QString &line) const
{
    SyntaxParsingState state;
    state.groups = {{SyntaxRule::GROUP_LINE, Token(Range(0, line.size()), "")}};
    state.tokens = {};

    for (const auto &rule : rules_) {
        rule.apply(line, state);
    }
    for (const auto &kv : state.groups) {
        const auto &token = kv.second;
        state.tokens.push_back(token);
    }
    state.tokens.sort([](const Token &x, const Token &y) {
        return x.range.start < y.range.start
               || (x.range.start == y.range.start && x.range.end > y.range.end);
    });
    return std::move(state.tokens);
}

SyntaxCollection::SyntaxCollection(const ConfigNode &node, bool runTests)
{
    for (const auto &kv : node.members()) {
        auto syntax = Syntax(kv.first, kv.second, runTests);
        syntaxes_.push_back(syntax);
    }
}

void SyntaxCollection::merge(const SyntaxCollection &other)
{
    for (const auto &syntax : other.syntaxes_) {
        for (auto iter = syntaxes_.begin(); iter != syntaxes_.end(); ++iter) {
            if (syntax.name() == iter->name()) {
                WARN << "Syntax" << syntax.nameAndLocation()
                     << "overrides syntax" << iter->nameAndLocation();
                iter = syntaxes_.erase(iter);
            }
        }
        syntaxes_.push_back(syntax);
    }
}

std::list<Token> SyntaxCollection::parse(const QString &line) const
{
    std::list<Token> result;
    for (const auto &syntax : syntaxes_) {
        result = syntax.parse(line);
        if (result.size() > 1)
            break;
    }
    return result;
}

using StringTokens = std::list<std::pair<QString, QString>>;

static StringTokens stringifyTokens(const QString &line,
                                    const std::list<Token> &tokens)
{
    StringTokens pairs;
    for (const auto &token : tokens) {
        auto str = line.mid(token.range.start, token.range.length());
        if (token.colorScope.isEmpty())
            continue;
        pairs.emplace_back(str, token.colorScope);
    }
    return pairs;
}

static StringTokens parseStringTokens(const ConfigNode &node)
{
    StringTokens pairs;
    for (const auto &elem : node.elements())
        pairs.push_back(std::make_pair(elem.element(0).asString(),
                                       elem.element(1).asString()));
    return pairs;
}

bool Syntax::runTest(const QString &line, const ConfigNode &node)
{
    auto tokens = parse(line);
    auto strTokens = stringifyTokens(line, tokens);
    auto expectedTokens = parseStringTokens(node);
    if (strTokens != expectedTokens) {
        StructStream ss;
        ss << BEGIN_MAP << "line" << line << "parsed" << strTokens << "expected"
           << expectedTokens << END_MAP;
        ERROR << "Syntax test" << node.location() << "failed:" << NOQUOTE
              << ss.toString();
        if (strTokens.size() != expectedTokens.size())
            ERROR << "Number of tokens does not match:" << strTokens.size()
                  << "vs" << expectedTokens.size();
        else {
            auto token1 = strTokens.begin();
            auto token2 = expectedTokens.begin();
            while (token1 != strTokens.end()) {
                if (*token1 != *token2)
                    ERROR << "Tokens does not match:" << *token1 << "vs"
                          << *token2;
                ++token1, ++token2;
            }
        }
        return false;
    }
    DEBUG << "Syntax test passed:" << node.location();
    return true;
}
