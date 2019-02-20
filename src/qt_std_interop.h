#pragma once

#include <QString>
#include <QStringList>

namespace std
{
    template <> struct hash<QString> {
        size_t operator()(const QString& str) const { return qHash(str); }
    };
}

template <typename Cont>
QStringList makeStringListFromValues(const Cont &cont)
{
    QStringList result;
    for (const auto &kv : cont) {
        result.push_back(kv.second);
    }
    return result;
}