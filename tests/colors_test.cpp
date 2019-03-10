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
                       nameAndColor.second.name().replace("#", "0x"));
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

TEST_F(ColorsTest, SingleEmpty)
{
    QString str = R"(
schemes:
    basic: {}
    )";

    auto parsed
        = ColorScheme::loadAll(ConfigNode::parseString(expandColors(str)));
    ColorScheme::Map expected({{"basic", ColorScheme()}});

    ASSERT_EQ(parsed, expected);
}

template <typename T, typename Cont>
std::map<std::string, T> convertToStdStringMap(const Cont &cont)
{
    std::map<std::string, T> result;
    for (const auto &kv : cont) {
        result.emplace(kv.first.toStdString(), kv.second);
    }
    return result;
}

TEST_F(ColorsTest, SingleColor)
{
    QString str = R"(
schemes:
    basic:
        base:
            Text: [$red, $blue]
    )";

    auto parsed
        = ColorScheme::loadAll(ConfigNode::parseString(expandColors(str)));
    ColorScheme::Map expected({{"basic", ColorScheme().setText({RED, BLUE})}});

    ASSERT_EQ(convertToStdStringMap<ColorScheme>(parsed),
              convertToStdStringMap<ColorScheme>(expected));
}