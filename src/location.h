/* TODO: header */

#pragma once

#include <QString>

class Location final {
  public:
    Location() = default;
    Location(const QString &path, unsigned lineNumber);
    const QString &path() const { return path_; }
    unsigned lineNumber() const { return lineNumber_; }

    QString toString() const;
    operator QString() const;

  private:
    QString path_;
    unsigned lineNumber_ = 0;
};

QDebug& operator<<(QDebug &debug, const Location &location);