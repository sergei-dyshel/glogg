#include "syntax.h"
#include "log.h"

struct SyntaxParsingState {
    std::unordered_map<QString, Token> groups;
    std::list<Token> tokens;
};

static const QString GROUP_LINE = "LINE";
static const QString GROUP_MATCH = "MATCH";
static const QString GROUP_GROUP = "GROUP";

template <> const QString Enum<SyntaxRule::SearchType>::name = "SearchType";
template <> const std::unordered_map<SyntaxRule::SearchType, QString>
    Enum<SyntaxRule::SearchType>::strings
    = {{SyntaxRule::SearchType::MATCH, "match"},
       {SyntaxRule::SearchType::ALL, "all"}};

static QString stringList2Regex(const QStringList &strings)
{
    QStringList escaped;
    for (const auto &str : strings)
        escaped.push_back(QRegularExpression::escape(str));
    return escaped.join('|');
}

SyntaxRule::SyntaxRule(const QString &name, const QString &group,
                       const QString &regExp,
                       const std::unordered_map<QString, QString> &colorize)
    : name_(name), matchGroup_(group), regExp_(regExp), colorize_(colorize)
{
    PostInit();
}

#define RULE_TRACE TRACE << "Rule" << fullName() << ":"

SyntaxRule::SyntaxRule(const ConfigNode &node)
{
    name_ = node.member("name", "").asString();
    matchGroup_ = node.member("group", GROUP_LINE).asString();
    auto regexNode = node.requiredMember("regex");
    QString regex = regexNode.isScalar()
                        ? regexNode.asString()
                        : stringList2Regex(regexNode.asStringList());
    regExp_ = QRegularExpression(regex);

    searchType_ = node.memberEnum<SearchType>("searchType", SearchType::MATCH);

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

SyntaxRule::Error::Error(const QString &ruleName, const LogContext &context)
    : Exception(context)
{
    QDEBUG_COMPAT(stream_.d);
    stream_.d.quote();
    stream_ << "Rule " << ruleName << ": ";
}

SyntaxRule::Error SyntaxRule::error(const LogContext &context) const
{
    return Error(fullName(), context);
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
            break;
        case SearchType::ALL:
            auto iter = regExp_.globalMatch(groupStr);
            while (iter.hasNext()) {
                processMatch(iter.next(), state);
            }
            break;
    }
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

Syntax::Syntax() { usedGroups_.insert(GROUP_LINE); }

Syntax::Syntax(const QString &name, const ConfigNode &node) : Syntax()
{
    name_ = name;
    DEBUG << "Loading syntax" << name_;
    for (const auto &elem : node.elements())
        addRule(SyntaxRule(elem));
    DEBUG << "Loaded " << rules_.size() << " rules";
}

Syntax &Syntax::addRule(const SyntaxRule &_rule)
{
    SyntaxRule rule = _rule;

    if (rule.name_.isEmpty()) {
        if (!usedNames_.count(rule.matchGroup_))
            rule.name_ = rule.matchGroup_;
        else {
            unsigned i = 1;
            do {
                rule.name_ = rule.matchGroup_ + QString::number(i++);
            } while (usedNames_.count(rule.name_));
        }
    }
    if (usedNames_.count(rule.name_))
        throw rule.error(HERE) << ": already exists";
    usedNames_.insert(rule.name_);

    if (!usedGroups_.count(rule.matchGroup_))
        throw rule.error(HERE) << "matches group" << rule.matchGroup_
                               << "which is not provided by previous rules";
    for (const auto &group : rule.regExpGroups_)
        if (group != GROUP_MATCH)
            usedGroups_.insert(group);
    for (const auto &kv : rule.colorize_) {
        auto group = kv.first;
        if (group != GROUP_GROUP && group != GROUP_MATCH
            && !usedGroups_.count(group))
            throw rule.error(HERE)
                << "colorizes group" << group
                << "that is not provided by this or previous rules";
        usedScopes_.insert(kv.second);
    }
    rule.parentName_ = name_;

    rules_.push_back(rule);
    return *this;
}

std::list<Token> Syntax::parse(const QString &line) const
{
    SyntaxParsingState state
        = {.groups = {{GROUP_LINE, Token(Range(0, line.size()), "")}},
           .tokens = {}};

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

SyntaxCollection::SyntaxCollection(const ConfigNode &node)
{
    for (const auto &kv : node.members()) {
        auto syntax = Syntax(kv.first, kv.second);
        addSyntax(syntax);
    }
}

void SyntaxCollection::addSyntax(const Syntax &syntax)
{
    if (usedNames_.count(syntax.name_))
        throw Exception(HERE)
            << "Syntax" << syntax.name_ << "already loaded (duplicate?)";
    usedNames_.insert(syntax.name_);
    syntaxes_.push_back(syntax);
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
