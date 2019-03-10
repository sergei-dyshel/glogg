#include "log.h"
#include "location.h"

Location::Location(const QString &path, unsigned lineNumber)
    : path_(path), lineNumber_(lineNumber)
{}

QDebug& operator<<(QDebug &debug, const Location &location)
{
    QDEBUG_COMPAT(debug);
    return debug << location.toString();
}

QString Location::toString() const
{
    return (path_.isEmpty() ? "<string>" : path_)
           + (lineNumber_ > 0 ? QString(":") + lineNumber_ : "");
}

Location::operator QString() const { return toString(); }
