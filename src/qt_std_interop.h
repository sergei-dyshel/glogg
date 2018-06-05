#pragma once

#include <ostream>
#include <functional>

#include <QString>
#include <QColor>

inline std::string QStringToStd(const QString &str) {
    return str.isNull() ? "" : str.toStdString();
}

inline std::ostream& operator<<(std::ostream& os, const QString& str)
{
    return os << QStringToStd(str);
}

inline std::ostream& operator<<(std::ostream& os, const QColor& color) {
    return os << color.name();
}

namespace std
{
    template <> struct hash<QString> {
        size_t operator()(const QString& str) const { return qHash(str); }
    };
}