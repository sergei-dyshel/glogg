// TODO: add header

#include "color_scheme.h"

#include "log.h"

#include <yaml-cpp/yaml.h>

#include <QPalette>

TextColor::TextColor(const ConfigNode &node)
    : foreground(colorFromNode(node.element(0))),
      background(colorFromNode(node.element(1)))
{}

QColor TextColor::colorFromNode(const ConfigNode& node)
{
    auto str = node.asString();
    QColor color(str);
    if (!color.isValid())
        throw node.error(HERE) << " is not a valid color: " << str;
    return color;
}

const QString ColorScheme::TEXT("Text");
const QString ColorScheme::SELECTION("Selection");
const QString ColorScheme::QUICK_FIND("QuickFind");

ColorScheme::ColorScheme(const ConfigNode &node)
{
    const QPalette palette;

    text = TextColor(palette.text().color(), palette.base().color());
    selection = TextColor(palette.highlightedText().color(),
                                   palette.highlight().color());
    quickFind = TextColor(QColor("black"), QColor("yellow"));

    LOG(logDEBUG) << "Loading color scheme";
    if (node) {
        try {
            if (node.hasMember("defs")) {
                for (const auto &member : node.member("defs").members())
                    defs_.emplace(member.first,
                                  QColor(member.second.as<int>()));
            }
            if (node.hasMember("base")) {
                auto base = node.member("base");
                if (base.hasMember(TEXT))
                    text = readTextColor(base.member(TEXT));
                if (base.hasMember(SELECTION))
                    selection = readTextColor(base.member(SELECTION));
                if (base.hasMember(QUICK_FIND))
                    quickFind = readTextColor(base.member(QUICK_FIND));
            }
            if (node.hasMember("user")) {
                for (const auto &member : node.member("user").members())
                    user_.emplace(member.first, readColor(member.second, true));
            }
        }
        catch (const ConfigError& err) {
            LOG(logERROR) << err;
        }
    }

    DEBUG << TEXT << text;
    DEBUG << SELECTION << selection;
    DEBUG << QUICK_FIND << quickFind;

    for (const auto &kv : user_)
        DEBUG << kv.first << kv.second.name();
}

QColor ColorScheme::readColor(const ConfigNode& node, bool allowUser)
{
    if (node.is<QRgb>())
        return QColor(node.as<QRgb>());
    auto str = node.asString();
    if (allowUser && user_.count(str))
        return user_.at(str);
    if (!defs_.count(str))
        throw node.error(HERE)
            << " has value of " << str
            << " but must be either 0xRGB or alias to existing color";
    return defs_.at(str);
}

TextColor ColorScheme::readTextColor(const ConfigNode& node)
{
    return TextColor(readColor(node.element(0), false),
                     readColor(node.element(1), false));
}
bool ColorScheme::hasScope(const QString &name) const
{
    return scopeColor(name, TextColor()) != TextColor();
}

TextColor ColorScheme::scopeColor(const QString& name) const {
    return scopeColor(name, text);
}

TextColor ColorScheme::scopeColor(const QString& name,
                             const TextColor& defaultColor) const
{
    if (name == TEXT)
        return text;
    if (name == SELECTION)
        return selection;
    if (name == QUICK_FIND)
        return quickFind;
    if (user_.count(name))
        return TextColor(user_.at(name), text.background);
    return defaultColor;
}

bool operator==(const TextColor &color1, const TextColor &color2)
{
    return color1.foreground == color2.foreground
           && color1.background == color2.background;
}

bool operator!=(const TextColor &color1, const TextColor &color2)
{
    return !(color1 == color2);
}

QDebug& operator<<(QDebug& debug, const TextColor& color)
{
    QDEBUG_COMPAT(debug);
    return debug << "{foreground: " << color.foreground.name()
                 << ", background: " << color.background.name() << "}";
}