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
        SyntaxRule("rule1", "line", ".*", {{"line", BASE}}));
    Check("some string", {{"some string", BASE}},
          {{"some string", BASE}});
}

TEST_F(SyntaxTest, OneRule)
{
    syntax_.addRule(SyntaxRule("rule1", "line", "(?<num>\\d+)",
                               {{"line", BASE}, {"num", NUM}}));
    Check("prefix1234suffix", {{"prefix1234suffix", BASE}, {"1234", NUM}},
          {{"prefix", BASE}, {"1234", NUM}, {"suffix", BASE}});
}

TEST_F(SyntaxTest, Nested)
{
    syntax_.addRule(SyntaxRule("rule1", "line", "(?<num>\\d+) (?<rest>.*)",
                               {{"num", NUM}}));
    syntax_.addRule(SyntaxRule("rule2", "rest", "\\[(?<cat>.*)\\]",
                               {{"line", BASE}, {"cat", CAT}}));
    Check("1234 [category] suffix",

          {{"1234 [category] suffix", BASE}, {"1234", NUM}, {"category", CAT}},

          {{"1234", NUM}, {" [", BASE}, {"category", CAT}, {"] suffix", BASE}});
}