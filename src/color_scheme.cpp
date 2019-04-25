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

#include "color_scheme.h"

#include "log.h"
#include "template_utils.h"
#include "struct_stream.h"

#include <yaml-cpp/yaml.h>

#include <QPalette>
#include <QRegularExpression>

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

const QString ColorScheme::TEXT("text");
const QString ColorScheme::SELECTION("selection");
const QString ColorScheme::QUICK_FIND("quickFind");
const QString ColorScheme::HIGHLIGHT("highlight");

const unsigned ColorScheme::HIGHLIGHT_COUNT = 4;

ColorScheme::ColorScheme(const QString &name, const Location &location)
    : highlight_{"red", "blue", "green", "yellow"}, name_(name),
      location_(location)
{
    const QPalette palette;

    text = TextColor(palette.text().color(), palette.base().color());
    selection = TextColor(palette.highlightedText().color(),
                                   palette.highlight().color());
    quickFind = TextColor(QColor("black"), QColor("yellow"));
    generateColorsMap();
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

ColorScheme &ColorScheme::addUser(const User &user, bool override_)
{
    mergeMaps(user_, user, override_);
    return *this;
}

void ColorScheme::addDefs(const ConfigNode& node, Defs &defs)
{
    Defs newDefs;
    if (!node.hasMember("defs"))
        return;
    for (const auto &member : node.requiredMember("defs").members()) {
        newDefs.emplace(member.first, readColor(member.second, &defs));
    }
    mergeMaps(defs, newDefs, true /* override */);
}

void ColorScheme::addScopes(const ConfigNode &node,
                      const Defs &defs)
{
    if (node.hasMember(TEXT))
        text = readTextColor(node.requiredMember(TEXT), defs);
    if (node.hasMember(SELECTION))
        selection = readTextColor(node.requiredMember(SELECTION), defs);
    if (node.hasMember(QUICK_FIND))
        quickFind = readTextColor(node.requiredMember(QUICK_FIND), defs);
    if (node.hasMember(HIGHLIGHT)) {
        auto hlNode = node.requiredMember(HIGHLIGHT);
        if (hlNode.numElements() != HIGHLIGHT_COUNT)
            throw hlNode.error(HERE)
                << "Must define" << HIGHLIGHT_COUNT << "highlight colors";
        for (unsigned i = 0;i < HIGHLIGHT_COUNT; ++i)
            highlight_[i] = readColor(hlNode.element(i), &defs);
    }
    if (node.hasMember("user")) {
        for (const auto &member : node.requiredMember("user").members())
            user_.emplace(member.first,
                          readColor(member.second, &defs, &user_));
    }
}

auto ColorScheme::loadAll(const ConfigNode &node) -> Map
{
    Map schemes;
    std::map<QString, QColor> defs;

    const auto &allSchemeNodes = node.members();
    if (allSchemeNodes.empty())
        WARN << "No color schemes defined in" << node.location().path();
    for (const auto &pair : allSchemeNodes) {
        const auto &name = pair.first;
        if (name.startsWith("_")) {
            DEBUG << "Skipping abstract scheme " << name;
            continue;
        }
        const auto *schemeNode = &pair.second;
        schemeNode->assertProperties({"defs", "inherits", "text", "selection",
                                      "quickFind", "highlight", "user"});
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
        defs.clear();
        for (const auto &nodeToAdd : nodesToAdd)
            scheme.addDefs(*nodeToAdd, defs);
        for (const auto &nodeToAdd : nodesToAdd)
            scheme.addScopes(*nodeToAdd, defs);
        scheme.generateColorsMap();
        schemes.emplace(name, scheme);
        DEBUG << "Added scheme" << name << ":" << scheme;
    }
    return schemes;
}

QColor ColorScheme::readColor(const ConfigNode &node, const Defs *defs,
                              const User *userScopes)
{
    auto str = node.asString();
    if (str.contains(QRegularExpression("^#[0-9a-fA-F]{6}$")))
        return QColor(str);
    if (defs == nullptr)
        throw node.error(HERE)
            << "has value of" << str << "but must be of format #RRGGBB";
    if (defs->count(str))
        return defs->at(str);
    if (userScopes && userScopes->count(str))
        return userScopes->at(str);
    throw node.error(HERE)
        << "has value of" << str
        << "but must be either string #RRGGBB or alias to existing color";
}

TextColor ColorScheme::readTextColor(const ConfigNode& node, const Defs &defs)
{
    return TextColor(readColor(node.element(0), &defs),
                     readColor(node.element(1), &defs));
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
    auto iter = colors_.find(name);
    if (iter == colors_.end())
        return defaultColor;
    return iter->second;
}

TextColor ColorScheme::withTextBackground(const QColor &foreground) const
{
    return TextColor(foreground, text.background);
}

QString ColorScheme::highlightScope(unsigned i) const
{
    return "highlight" + QString(i);
}


TextColor ColorScheme::highlightColor(unsigned i) const
{
    return withTextBackground(highlight_.at(i));
}

void ColorScheme::generateColorsMap()
{
    colors_.clear();
    colors_[TEXT] = text;
    colors_[SELECTION] = selection;
    colors_[QUICK_FIND] = quickFind;
    for (unsigned i = 0; i < HIGHLIGHT_COUNT; ++i)
        colors_[highlightScope(i)] = highlightColor(i);
    for (const auto &name_color: user_)
        colors_[name_color.first] = withTextBackground(name_color.second);
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

StructStream& operator<<(StructStream &ss, const ColorScheme &scheme)
{
    ColorScheme defaultScheme;
    ss << BEGIN_MAP;
    if (scheme.text != defaultScheme.text)
        ss << "text" << scheme.text;
    if (scheme.selection != defaultScheme.selection)
        ss << "selection" << scheme.selection;
    if (scheme.quickFind != defaultScheme.quickFind)
        ss << "quickFind" << scheme.quickFind;
    if (!scheme.userScopes().empty())
        ss << "user" << scheme.userScopes();
    return ss << END_MAP;
}

StructStream& operator<<(StructStream& ss, const QColor& color)
{
    return ss << color.name();
}

StructStream& operator<<(StructStream& ss, const TextColor& color)
{
    return ss << BEGIN_SEQ << color.foreground << color.background << END_SEQ;
}