// TODO: add header

#pragma once

#include "range.h"

#include <iostream>
#include <list>
#include <vector>

#include <QString>
#include <QDebug>

class LogStream;

struct Token {
    Token() = default;
    Token(const Range &range_, const QString &colorScope_)
        : range(range_), colorScope(colorScope_)
    {}

    Token(const Token &token)
        : range(token.range), colorScope(token.colorScope)
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


void mergeTokens(std::list<Token> &upperTokens,
                    const std::list<Token> &lowerTokens);

void mergeSyntaxTokens(std::list<Token> &upperTokens,
                       const std::list<Token> &syntaxTokens);
void addLowerToken(std::list<Token> &tokens, const Token& lower);