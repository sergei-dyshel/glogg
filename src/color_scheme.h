// TODO: add header

#pragma once

#include "range.h"
#include "fwd.h"
#include "log.h"
#include "config_node.h"
#include "qt_std_interop.h"

#include <iostream>

#include <QColor>
#include <QString>

#include <unordered_map>

struct TextColor final {
    // TODO: remove default after writing tests
    explicit TextColor(const QColor &fore = QColor(), const QColor &back = QColor())
        : foreground(fore), background(back)
    {}

    explicit TextColor(const ConfigNode& node);

    QColor foreground;
    QColor background;

private:
    static QColor colorFromNode(const ConfigNode& node);
}; // struct TextColor

class ColorScheme final {
  public:
    explicit ColorScheme(const ConfigNode& node = ConfigNode());

    TextColor scopeColor(const QString& name) const;
    TextColor scopeColor(const QString& name, const TextColor &defaultColor) const;
    bool hasScope(const QString &name) const;

    TextColor text;
    TextColor selection;
    TextColor quickFind;

    static const QString TEXT;
    static const QString SELECTION;
    static const QString QUICK_FIND;

  private:

    QColor readColor(const ConfigNode& node, bool allowUser);
    TextColor readTextColor(const ConfigNode& node);

    std::unordered_map<QString, QColor> defs_;
    std::unordered_map<QString, QColor> user_;
};

bool operator==(const TextColor &color1, const TextColor &color2);
// TODO: define using https://stackoverflow.com/questions/23388739/c-relational-operators-generator
bool operator!=(const TextColor &color1, const TextColor &color2);
QDebug& operator<<(QDebug &debug, const TextColor &color);