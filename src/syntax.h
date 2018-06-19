#pragma once

#include "colorizer.h"
#include "config_node.h"
#include "qt_std_interop.h"
#include "exception.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QRegularExpression>
#include <QString>

using SyntaxParsingState = std::unordered_map<QString, Token>;

class Syntax;
class SyntaxCollection;

class SyntaxRule final {
public:
    SyntaxRule(const ConfigNode &node);
    SyntaxRule(const QString &name, const QString &group, const QString &regExp,
               const std::unordered_map<QString, QString> &colorize);

    void apply(const QString &line, SyntaxParsingState& state) const;

    class Error : public Exception {
      public:
        Error(const QString &ruleName, const LogContext &context);
        DEFINE_EXCEPTION_SHIFT_OPERATOR(Error)
    };

    Error error(const LogContext &context) const;

    friend class Syntax;

    QString fullName() const { return parentName_ + "/" + name_; }

  private:

    void PostInit();

    QString name_;
    QString parentName_;
    QString matchGroup_;
    QRegularExpression regExp_;

    std::unordered_set<QString> regExpGroups_;

    std::unordered_map<QString, QString> colorize_;
};

class Syntax final {
public:
    Syntax();
    Syntax(const QString &name, const ConfigNode &node);

    std::list<Token> parse(const QString &line) const;

    Syntax &addRule(const SyntaxRule &rule);

    std::unordered_set<QString> usedScopes() const { return usedScopes_; }

    friend class SyntaxCollection;

private:
    QString name_;
    std::unordered_set<QString> usedScopes_;
    std::unordered_set<QString> usedGroups_;
    std::unordered_set<QString> usedNames_;
    std::vector<SyntaxRule> rules_;
};

class SyntaxCollection final {
public:
    SyntaxCollection() = default;
    SyntaxCollection(const ConfigNode &node);

    void addSyntax(const Syntax &syntax);

    std::list<Token> parse(const QString &line) const;

    const std::vector<Syntax> syntaxes() const { return syntaxes_; }

private:
    std::vector<Syntax> syntaxes_;
    std::unordered_set<QString> usedNames_;

};