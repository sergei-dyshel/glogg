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

#ifndef REGEXP_FILTER_H
#define REGEXP_FILTER_H

#include <QRegularExpression>
#include <QString>

#include "configuration.h"

class RegExpFilter final {
  public:
    RegExpFilter() = default;
    RegExpFilter( QString text, enum SearchRegexpType type = ExtendedRegexp,
                  bool case_insensitive = true );

    bool isValid() const { return include_.isValid() && exclude_.isValid(); }

    bool hasMatch( QString str ) const;

    QString errorMessage() const;

  private:
    static QString regExpErrorMsg( QString name,
                                   const QRegularExpression &regExp );

    static const QString separator_;

  private:
    QRegularExpression include_;
    QRegularExpression exclude_;
};

#endif /* REGEXP_FILTER_H */
