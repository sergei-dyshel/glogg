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
#include "struct_config.h"

class SyntaxTest : public ::testing::Test {
  public:
};

void verify(const QString &syntaxes)
{
    SyntaxCollection syntaxColl(ConfigNode::parseString(syntaxes), true /* run tests */);
}

TEST_F(SyntaxTest, WholeLine)
{
    verify(R"(
main:
    rules:
        - match: '.*'
          colorize:
            LINE: BASE
    tests:
        'some string':
            - ['some string', 'BASE']
    )");
}

TEST_F(SyntaxTest, OneRule)
{
    verify(R"#(
main:
    rules:
        - match: '(?<num>\d+)'
          colorize:
            LINE: BASE
            num: NUM
    tests:
        "prefix1234suffix":
            - ['prefix1234suffix', 'BASE']
            - ['1234', 'NUM']
    )#");
}

TEST_F(SyntaxTest, Nested)
{
    verify(R"#(
main:
    rules:
        - match: (?<num>\d+) (?<rest>.*)
          colorize:
            num: NUM
        - group: rest
          match: \[(?<cat>.*)\]
          colorize:
            LINE: BASE
            cat: CAT
    tests:
        "1234 [category] suffix":
            - ['1234 [category] suffix', BASE]
            - ['1234', NUM]
            - ['[category] suffix', rest]
            - ['category', CAT]
    )#");
}

TEST_F(SyntaxTest, MatchAll)
{
    verify(R"#(
main:
    rules:
        - matchAll: (?<name>\w+)=(?<num>\d+)
          colorize:
            name: BASE
            num: NUM
    tests:
        "name1=13 name2=15":
            - ['name1', BASE]
            - ['13', NUM]
            - ['name2', BASE]
            - ['15', NUM]
    )#");
}