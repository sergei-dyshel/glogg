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

#include "gtest/gtest.h"

#include "syntax.h"

static constexpr auto NUM = "NUM";
static constexpr auto BASE = "BASE";
static constexpr auto CAT = "CAT";


class SyntaxTest : public ::testing::Test {
public:
  using PairList = std::list<std::pair<std::string, std::string>>;
  void SetUp();
  void Check(const std::string &line,
             const PairList &syntax,
             const PairList &flat);
  Syntax syntax_;
  const QString LINE = SyntaxRule::GROUP_LINE;
  const SyntaxRule::SearchType MATCH = SyntaxRule::SearchType::MATCH;

private:
  PairList tokens2pairs(const std::string &line,
                         const std::list<Token> &tokens);
};

void SyntaxTest::SetUp()
{
    syntax_ = Syntax();
}

SyntaxTest::PairList SyntaxTest::tokens2pairs(const std::string &line,
                                              const std::list<Token> &tokens)
{
    PairList pairs;
    for (const auto &token : tokens) {
        auto str = line.substr(token.range.start, token.range.length());
        if (token.colorScope.isEmpty())
            continue;
        pairs.emplace_back(str, token.colorScope.toStdString());
    }
    return pairs;
}

void SyntaxTest::Check(const std::string &line,
                       const PairList &expectedSyntax,
                       const PairList &expectedFlat)
{
    auto qline = QString::fromStdString(line);
    auto syntaxTokens = syntax_.parse(qline);
    auto syntax = tokens2pairs(line, syntaxTokens);
    ASSERT_EQ(syntax, expectedSyntax);

    std::list<Token> flatTokens;
    mergeSyntaxTokens(flatTokens, syntaxTokens);
    auto flat = tokens2pairs(line, flatTokens);
    ASSERT_EQ(flat, expectedFlat);
}

TEST_F(SyntaxTest, Empty)
{
    Check("", {}, {});
}

TEST_F(SyntaxTest, WholeLine)
{
    syntax_.addRule(
        SyntaxRule("rule1", LINE, ".*", MATCH, {{LINE, BASE}}));
    Check("some string", {{"some string", BASE}},
          {{"some string", BASE}});
}

TEST_F(SyntaxTest, OneRule)
{
    syntax_.addRule(SyntaxRule("rule1", LINE, "(?<num>\\d+)", MATCH,
                               {{LINE, BASE}, {"num", NUM}}));
    Check("prefix1234suffix", {{"prefix1234suffix", BASE}, {"1234", NUM}},
          {{"prefix", BASE}, {"1234", NUM}, {"suffix", BASE}});
}

TEST_F(SyntaxTest, Nested)
{
    syntax_.addRule(SyntaxRule("rule1", LINE, "(?<num>\\d+) (?<rest>.*)", MATCH,
                               {{"num", NUM}}));
    syntax_.addRule(SyntaxRule("rule2", "rest", "\\[(?<cat>.*)\\]", MATCH,
                               {{LINE, BASE}, {"cat", CAT}}));
    Check("1234 [category] suffix",

          {{"1234 [category] suffix", BASE},
           {"1234", NUM},
           {"[category] suffix", "rest"},
           {"category", CAT}},

          {{"1234", NUM},
           {" ", BASE},
           {"[", "rest"},
           {"category", CAT},
           {"] suffix", "rest"}});
}