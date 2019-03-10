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
#include <utility>

using namespace std::rel_ops;

struct TextColor final {
    // TODO: remove default after writing tests
    TextColor(const QColor &fore = QColor(), const QColor &back = QColor())
        : foreground(fore), background(back)
    {}

    TextColor(const std::initializer_list<QColor> &init_list);

    explicit TextColor(const ConfigNode& node);

    QColor foreground;
    QColor background;

private:
    static QColor colorFromNode(const ConfigNode& node);
}; // struct TextColor

class ColorScheme final {
  public:
    using User = std::unordered_map<QString, QColor>;
    using Map = std::map<QString, ColorScheme>;

    explicit ColorScheme(const QString &name = "",
                         const Location &location = Location());

    // Used for tests
    ColorScheme &setText(const TextColor &text);
    ColorScheme &setSelection(const TextColor &selection);
    ColorScheme &setQuickFind(const TextColor &quickFind);
    ColorScheme &addUser(const User &user);

    static Map loadAll(const ConfigNode &node);

    const QString &name() const { return name_; }

    TextColor scopeColor(const QString& name) const;
    TextColor scopeColor(const QString &name,
                         const TextColor &defaultColor) const;
    bool hasScope(const QString &name) const;

    const User &userScopes() const { return user_; }

    TextColor text;
    TextColor selection;
    TextColor quickFind;

    static const QString TEXT;
    static const QString SELECTION;
    static const QString QUICK_FIND;

    bool operator==(const ColorScheme &other) const;

  private:
    using Defs = std::map<QString, QColor>;
    void add(const ConfigNode& node, const Defs &defs);

    QColor readColor(const ConfigNode& node, const Defs &defs, bool allowUser);
    TextColor readTextColor(const ConfigNode& node, const Defs &defs);

    QString name_;
    Location location_;
    User user_;
};

bool operator==(const TextColor &color1, const TextColor &color2);
QDebug& operator<<(QDebug &debug, const ColorScheme &scheme);
QDebug& operator<<(QDebug &debug, const TextColor &color);

DEFINE_STREAM_SHIFT_WITH_QDEBUG(ColorScheme)