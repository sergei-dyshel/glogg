#include "colorizer.h"
#include "log.h"
#include "qt_std_interop.h"

void mergeTokens(std::list<Token> &upperTokens,
                    const std::list<Token> &lowerTokens)
{
    auto upper = upperTokens.begin();
    auto lower = lowerTokens.begin();
    if (lowerTokens.empty())
       return;
    unsigned lower_start = lower->range.start;

    while (lower != lowerTokens.end()) {
        lower_start = qMax(lower->range.start, lower_start);

        if (upper == upperTokens.end()) {
            upperTokens.push_back(
                Token(Range(lower_start, lower->range.end), lower->colorScope));
            ++lower;
            continue;
        }

        if (upper->range.start <= lower_start) {
            if (upper->range.end >= lower->range.end) {
                ++lower;
                continue;
            }
            lower_start = upper->range.end;
            if (upper != upperTokens.end())
                ++upper;
            continue;
        }
        if (upper->range.start >= lower->range.end) {
            upperTokens.insert(upper, *lower);
            ++lower;
            continue;
        }
        upperTokens.insert(
            upper,
            Token(Range(lower_start, upper->range.start), lower->colorScope));
        lower_start = upper->range.start;
    }
}

void mergeSyntaxTokens(std::list<Token> &upperTokens,
                       const std::list<Token> &syntaxTokens)
{
    for (auto iter = syntaxTokens.crbegin(); iter != syntaxTokens.crend();
         ++iter)
        addLowerToken(upperTokens, *iter);
}

void addLowerToken(std::list<Token> &upperTokens, const Token& lower)
{
    unsigned lower_start = lower.range.start;

    auto upper  = upperTokens.begin();

    for (; upper != upperTokens.end(); ++upper) {
        if (upper == upperTokens.end())
            break;

        if (upper->range.start <= lower_start) {
            if (upper->range.end >= lower.range.end)
                return;
            lower_start = qMax(upper->range.end, lower_start);
            continue;
        }

        if (upper->range.start >= lower.range.end)
          break;

        upperTokens.insert(
            upper,
            Token(Range(lower_start, upper->range.start), lower.colorScope));
        lower_start = upper->range.end;
    }

    if (lower_start < lower.range.end) {
        upperTokens.insert(
            upper, Token(Range(lower_start, lower.range.end), lower.colorScope));
    }
}
