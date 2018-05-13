// TODO: add header
#ifndef REGEXP_FILTER_H
#define REGEXP_FILTER_H

#include <QRegularExpression>
#include <QString>

#include "configuration.h"

class RegExpFilter final {
  public:
    RegExpFilter() = default;
    RegExpFilter( QString text, enum SearchRegexpType type,
                  bool case_insensitive );

    bool isValid() const { return include_.isValid() && exclude_.isValid(); }

    bool hasMatch( QString str ) const;

    RegExpFilter &operator=( const RegExpFilter &other );

    QString errorMessage() const;

  private:
    static QString regExpErrorMsg( QString name,
                                   const QRegularExpression &regExp );

    static const QString separator_;

  private:
    QString includeText_ = "";
    QString excludeText_ = "";
    bool hasSeparator_ = false;
    QRegularExpression include_;
    QRegularExpression exclude_;
};

#endif /* REGEXP_FILTER_H */