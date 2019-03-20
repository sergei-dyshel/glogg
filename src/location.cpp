#include "log.h"
#include "location.h"

#include <QFileInfo>

Location::Location(const QString &path, unsigned lineNumber)
    : path_(path), lineNumber_(lineNumber)
{}

QDebug& operator<<(QDebug &debug, const Location &location)
{
    QDEBUG_COMPAT(debug);
    return debug << location.toString();
}

QString Location::toString(bool fileNameOnly) const
{
    return (path_.isEmpty() ? "<string>" : (fileNameOnly ? fileName() : path_))
           + (lineNumber_ > 0 ? (QString(":") + QString::number(lineNumber_))
                              : "");
}

QString Location::toShortString() const {
    return toString(true /* filename only */);
}

Location::operator QString() const { return toString(); }

QString Location::fileName() const
{
    return QFileInfo(path_).fileName();
}