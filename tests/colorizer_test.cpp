#include "gtest/gtest.h"

#include "colorizer.h"
#include "color_scheme.h"

#include <iostream>
#include <tuple>

class ColorizerTest : public ::testing::Test {
public:
  ColorizerTest() {}

  using Tuple = std::tuple<unsigned, unsigned, QString>;
  using TupleList = std::list<Tuple>;

  void Check(TupleList upperTuples, TupleList lowerTuples,
             TupleList resultTuples)
  {
      std::list<Token> upperTokens;
      for (auto tuple : upperTuples)
          upperTokens.push_back(tuple2token(tuple));

      std::list<Token> lowerTokens;
      for (auto tuple : lowerTuples)
          lowerTokens.push_back(tuple2token(tuple));

      mergeSyntaxTokens(upperTokens, lowerTokens);

    for (auto token: upperTokens)
        DEBUG << token;

      std::list<Token> expectedTokens;
      for (auto tuple : resultTuples)
          expectedTokens.push_back(tuple2token(tuple));

      ASSERT_EQ(expectedTokens, upperTokens);
    }

    Token tuple2token(Tuple tuple) {
        return Token(Range(std::get<0>(tuple), std::get<1>(tuple)),
                     std::get<2>(tuple));
    }

    const QString SEL= ColorScheme::SELECTION;
    const QString QF = ColorScheme::QUICK_FIND;
    const QString TXT = ColorScheme::TEXT;

}; // class ColorizerTest

TEST_F(ColorizerTest, Basic) {
    Check({{4, 9, SEL}},
          {{0, 5, QF}, {7, 10, TXT}},

            {{0, 4, QF}, {4, 9, SEL}, {9, 10, TXT}});
}

TEST_F(ColorizerTest, OneUpperManyLower) {
    Check({{3, 9, SEL}},
          {{0, 4, QF}, {5, 6, TXT}, {7, 10, TXT}},
            {{0, 3, QF}, {3, 9, SEL}, {9, 10, TXT}});
}

TEST_F(ColorizerTest, ManyUpperOneLower)
{
    Check({{3, 5, SEL}, {6, 8, SEL}, {9, 10, SEL}}, {{0, 11, QF}},

          {{0, 3, QF},

           {3, 5, SEL},
           {5, 6, QF},
           {6, 8, SEL},
           {8, 9, QF},
           {9, 10, SEL},
           {10, 11, QF}});
}

TEST_F(ColorizerTest, SameRange)
{
    Check({{4, 10, SEL}}, {{4, 10, QF}}, {{4, 10, SEL}});
}

TEST_F(ColorizerTest, Test)
{
    Check({{15, 19, SEL}},
          {{21, 37, QF}, {38, 40, TXT}},
          {{15, 19, SEL}, {21, 37, QF}, {38, 40, TXT}});
}

TEST_F(ColorizerTest, FullyCover)
{
    Check({{3, 6, SEL}, {6, 9, SEL}}, {{3, 8, QF}, {8, 9, QF}},
          {{3, 6, SEL}, {6, 9, SEL}});
}