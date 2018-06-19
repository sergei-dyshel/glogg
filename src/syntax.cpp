#include "syntax.h"
#include "log.h"

SyntaxRule::SyntaxRule(const QString &name, const QString &group,
                       const QString &regExp,
                       const std::unordered_map<QString, QString> &colorize)
    : name_(name), matchGroup_(group), regExp_(regExp), colorize_(colorize)

{
    PostInit();
}

SyntaxRule::SyntaxRule(const ConfigNode &node)
{
    name_ = node.member("name").asString();
    matchGroup_ = node.member("group").asString();
    regExp_ = QRegularExpression(node.member("regex").asString());
    if (node.hasMember("colorize"))
        for (const auto &colorRule : node.member("colorize").members())
            colorize_.emplace(colorRule.first, colorRule.second.asString());

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

    for (auto group : regExp_.namedCaptureGroups())
        if (group != "")
            regExpGroups_.insert(group);
}

#define RULE_TRACE TRACE << "Rule" << fullName() << ":"

void SyntaxRule::apply(const QString &line, SyntaxParsingState& state) const
{
    RULE_TRACE << "Applying to state" << state;
    auto it = state.find(matchGroup_);
    if (it == state.end())
        return;

    auto range = it->second.range;
    QStringRef groupStr(&line, range.start, range.length());

    RULE_TRACE << "Matching" << regExp_.pattern() << "on" << groupStr;

    auto match = regExp_.match(groupStr);
    if (!match.hasMatch())
        return;

    for (const auto &group : regExpGroups_) {
        auto captured = match.capturedRef(group);
        RULE_TRACE << "captured" << captured << "as" << group;
        if (captured.isNull() || captured.isEmpty())
            continue;
        Token token(Range::WithLength(captured.position(), captured.length()),
                    group);
        state[group] = token;
    }

    for (const auto &kv : colorize_)
        if (state.count(kv.first))
            state.at(kv.first).colorScope = kv.second;
}

Syntax::Syntax() { usedGroups_.insert("line"); }

Syntax::Syntax(const QString &name, const ConfigNode &node) : Syntax()
{
    name_ = name;
    DEBUG << "Loading syntax" << name_;
    for (const auto &elem : node.elements())
        addRule(SyntaxRule(elem));
    DEBUG << "Loaded " << rules_.size() << " rules";
}

Syntax &Syntax::addRule(const SyntaxRule &rule)
{
    if (usedNames_.count(rule.name_))
        throw rule.error(HERE) << ": already exists";
    usedNames_.insert(rule.name_);

    if (!usedGroups_.count(rule.matchGroup_))
        throw rule.error(HERE) << "matches group" << rule.matchGroup_
                               << "which is not provided by previous rules";
    usedGroups_.insert(rule.regExpGroups_.cbegin(), rule.regExpGroups_.cend());
    for (const auto &kv : rule.colorize_) {
        auto group = kv.first;
        if (!usedGroups_.count(group))
            throw rule.error(HERE)
                << "colorizes group" << group
                << "that is not provided by this or previous rules";
        usedScopes_.insert(kv.second);
    }
    rules_.push_back(rule);
    rules_.back().parentName_ = name_;
    return *this;
}

std::list<Token> Syntax::parse(const QString &line) const
{
    std::list<Token> result;
    SyntaxParsingState state = {{"line", Token(Range(0, line.size()), "")}};
    for (const auto &rule : rules_) {
        rule.apply(line, state);
    }
    for (const auto &kv : state) {
        const auto &token = kv.second;
        result.push_back(token);
    }
    result.sort([](const Token &x, const Token &y) {
        return x.range.start < y.range.start
               || (x.range.start == y.range.start && x.range.end > y.range.end);
    });
    return result;
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
