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

#include "color_scheme.h"

static const QColor RED = "red";
static const QColor BLUE = "blue";
static const QColor WHITE = "white";
static const QColor BLACK = "black";

static const std::map<QString, QColor> ALL_COLORS
    = {{"white", WHITE}, {"black", BLACK}, {"red", RED}, {"blue", BLUE}};

static QString expandColors(const QString &templ)
{
    QString result = templ;
    for (const auto &nameAndColor : ALL_COLORS) {
        result.replace("$" + nameAndColor.first,
                       "'" + nameAndColor.second.name() + "'");
    }
    return result;
}

class ColorsTest : public ::testing::Test {
public:
    void SetUp() {
        for (const auto &nameAndColor : ALL_COLORS) {
            ASSERT_TRUE(nameAndColor.second.isValid());
        }
    }
};

template <typename T, typename Cont>
std::map<std::string, T> convertToStdStringMap(const Cont &cont)
{
    std::map<std::string, T> result;
    for (const auto &kv : cont) {
        result.emplace(kv.first.toStdString(), kv.second);
    }
    return result;
}

void verify(const QString &str, const ColorScheme::Map &expected)
{
    auto parsed
        = ColorScheme::loadAll(ConfigNode::parseString(expandColors(str)));
    auto parsedConverted = convertToStdStringMap<ColorScheme>(parsed);
    auto expectedConverted = convertToStdStringMap<ColorScheme>(expected);

    ASSERT_EQ(parsedConverted, expectedConverted);
}

void verify_throws(const QString &str)
{
    ASSERT_ANY_THROW(
        ColorScheme::loadAll(ConfigNode::parseString(expandColors(str))));
}

void verifyIdentical(const QString &str, const QString &expectedStr)
{
    auto expected = ColorScheme::loadAll(
        ConfigNode::parseString(expandColors(expectedStr)));
    verify(str, expected);
}

void verifyError(const QString &str)
{
    ASSERT_ANY_THROW(
        ColorScheme::loadAll(ConfigNode::parseString(expandColors(str))));
}

TEST_F(ColorsTest, SingleEmpty)
{
    QString str = R"(
basic: {}
    )";

    verify(str, {{"basic", ColorScheme()}});
}

TEST_F(ColorsTest, SingleColor)
{
    QString str = R"(
basic:
    text: [$red, $blue]
    )";

    verify(str, {{"basic", ColorScheme().setText({RED, BLUE})}});
}

TEST_F(ColorsTest, UnexpectedProperties)
{
    verify_throws(R"(
basic:
    text: [$red, $blue]
    unexpected: $red
    )");
}


TEST_F(ColorsTest, DefinitionsAndUserScopes)
{
    QString str = R"(
basic:
    defs:
        red: $red
        blue: $blue
    text: [red, blue]
    user:
        scope1: red
        scope2: blue
    )";

    verify(str, {{"basic", ColorScheme()
                               .setText({RED, BLUE})
                               .addUser({{"scope1", RED}, {"scope2", BLUE}})}});
}

TEST_F(ColorsTest, MissingDefinition)
{
    QString str = R"(
basic:
    defs:
        red: $red
    text: [red, blue]
    )";

    verifyError(str);
}

TEST_F(ColorsTest, InheritBasic)
{
    QString str1 = R"(
_base:
    text: [$white, $black]
    quickFind: [$red, $blue]
    user:
        scope1: $white
        scope2: $black
basic:
    inherits: _base
    text: [$red, $blue]
    user:
        scope1: $blue
        scope3: $red
    )";

    QString str2 = R"(
basic:
    text: [$red, $blue]
    quickFind: [$red, $blue]
    user:
        scope1: $white
        scope2: $black
        scope3: $red
    )";

    verifyIdentical(str1, str2);
}

TEST_F(ColorsTest, InheritOverrideBasic)
{
    QString str1 = R"(
normal:
    defs:
        red: $red
        blue: $blue
    user:
        scope1: red
        scope2: blue
revert:
    inherits: normal
    defs:
        red: blue
        blue: red
    )";

    QString str2 = R"(
normal:
    user:
        scope1: $red
        scope2: $blue
revert:
    user:
        scope1: $blue
        scope2: $red
    )";

    verifyIdentical(str1, str2);
}