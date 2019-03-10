// TODO: add header

#include "color_scheme.h"

#include "log.h"
#include "template_utils.h"

#include <yaml-cpp/yaml.h>

#include <QPalette>

TextColor::TextColor(const ConfigNode &node)
    : foreground(colorFromNode(node.element(0))),
      background(colorFromNode(node.element(1)))
{}

TextColor::TextColor(const std::initializer_list<QColor> &init_list)
{
    auto iter = init_list.begin();
    if (iter == init_list.end())
        throw ASSERT
            << "Initializer list for TextColor must be of length 1 or 2";
    foreground = *iter;
    ++iter;
    if (iter == init_list.end())
        return;
    background = *iter;
    ++iter;
    if (iter != init_list.end())
        throw ASSERT
            << "Initializer list for TextColor must be of length 1 or 2";
}

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

ColorScheme::ColorScheme(const QString &name, const Location &location)
    : name_(name), location_(location)
{
    const QPalette palette;

    text = TextColor(palette.text().color(), palette.base().color());
    selection = TextColor(palette.highlightedText().color(),
                                   palette.highlight().color());
    quickFind = TextColor(QColor("black"), QColor("yellow"));
}

ColorScheme &ColorScheme::setText(const TextColor &text)
{
    this->text = text;
    return *this;
}

ColorScheme &ColorScheme::setSelection(const TextColor &selection)
{
    this->selection = selection;
    return *this;
}

ColorScheme &ColorScheme::setQuickFind(const TextColor &quickFind)
{
    this->quickFind = quickFind;
    return *this;
}

void ColorScheme::add(const ConfigNode &node,
                      const Defs &defs)
{
    if (node.hasMember("base")) {
        auto base = node.requiredMember("base");
        DEBUG << base.toString() << base.requiredMember(TEXT);
        if (base.hasMember(TEXT))
            text = readTextColor(base.requiredMember(TEXT), defs);
        if (base.hasMember(SELECTION))
            selection = readTextColor(base.requiredMember(SELECTION), defs);
        if (base.hasMember(QUICK_FIND))
            quickFind = readTextColor(base.requiredMember(QUICK_FIND), defs);
    }
    if (node.hasMember("user")) {
        for (const auto &member : node.requiredMember("user").members())
            user_.emplace(member.first,
                          readColor(member.second, defs, true /* allowUser */));
    }

    DEBUG << TEXT << text;
    DEBUG << SELECTION << selection;
    DEBUG << QUICK_FIND << quickFind;

    for (const auto &kv : user_)
        DEBUG << kv.first << kv.second.name();
}

auto ColorScheme::loadAll(const ConfigNode &node) -> Map
{
    Map schemes;
    std::map<QString, QColor> defs;

    if (node.hasMember("defs")) {
        for (const auto &member : node.requiredMember("defs").members())
            defs.emplace(member.first, QColor(member.second.as<int>()));
    }

    if (!node.hasMember("schemes"))
        WARN << "No color schemes defined in" << node.location().path();

    const auto &allSchemeNodes = node.requiredMember("schemes").members();
    for (const auto &pair : allSchemeNodes) {
        const auto &name = pair.first;
        if (name.startsWith("_")) {
            DEBUG << "Skipping abstract scheme " << name;
            continue;
        }
        const auto *schemeNode = &pair.second;
        ColorScheme scheme(name, schemeNode->location());
        std::list<const ConfigNode *> nodesToAdd = {schemeNode};
        while (schemeNode->hasMember("inherits")) {
            const auto &base = schemeNode->requiredMember("inherits").asString();
            if (!allSchemeNodes.count(base))
                throw schemeNode->error(HERE)
                    << "Base scheme " << base << " not found";
            schemeNode = &allSchemeNodes.at(base);
            nodesToAdd.push_front(schemeNode);
        }
        for (const auto &nodeToAdd : nodesToAdd)
            scheme.add(*nodeToAdd, defs);
        schemes.emplace(name, scheme);
    }
    return schemes;
}

QColor ColorScheme::readColor(const ConfigNode &node, const Defs &defs,
                              bool allowUser)
{
    if (node.is<QRgb>()) {
        return QColor(node.as<QRgb>());
    }
    auto str = node.asString();
    if (allowUser && user_.count(str))
        return user_.at(str);
    if (!defs.count(str))
        throw node.error(HERE)
            << " has value of " << str
            << " but must be either 0xRGB or alias to existing color";
    return defs.at(str);
}

TextColor ColorScheme::readTextColor(const ConfigNode& node, const Defs &defs)
{
    return TextColor(
        readColor(node.element(0), defs, false /* do not allowUser */),
        readColor(node.element(1), defs, false));
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

bool ColorScheme::operator==(const ColorScheme &other) const
{
    return text == other.text && selection == other.selection
           && quickFind == other.quickFind && user_ == other.user_;
}

bool operator==(const TextColor &color1, const TextColor &color2)
{
    return color1.foreground == color2.foreground
           && color1.background == color2.background;
}

QDebug& operator<<(QDebug &debug, const ColorScheme &scheme)
{
    QDEBUG_COMPAT(debug);
    ColorScheme defaultScheme;
    if (scheme.text != defaultScheme.text)
        debug << "text: " << scheme.text;
    if (scheme.selection != defaultScheme.selection)
            debug << "selection: " << scheme.selection;
    if (scheme.quickFind != defaultScheme.quickFind)
            debug << "quickFind: " << scheme.quickFind;
    if (!scheme.userScopes().empty())
        debug << "user: " << scheme.userScopes();
    return debug;
}


QDebug& operator<<(QDebug& debug, const TextColor& color)
{
    QDEBUG_COMPAT(debug);
    return debug << "{fg: " << color.foreground.name()
                 << ", bg: " << color.background.name() << "}";
}