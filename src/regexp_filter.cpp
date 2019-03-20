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

#include "regexp_filter.h"

#include <QTextLayout>
#include <QLineEdit>
#include <QCoreApplication>

const QString RegExpFilter::separator_ = "|||";

RegExpFilter::RegExpFilter( QString text, enum SearchRegexpType type,
                            bool case_insensitive )
{
    if ( type == FixedString ) {
        include_.setPattern( QRegularExpression::escape( text ) );
        return;
    }
    auto parts = text.split( separator_ );
    if ( parts.size() >= 1 ) {
        includeText_ = parts.at( 0 );
    }
    if ( parts.size() >= 2 ) {
        hasSeparator_ = true;
        excludeText_ = parts.at( 1 );
    }

    auto opt = QRegularExpression::OptimizeOnFirstUsageOption
               | QRegularExpression::DontCaptureOption
               | QRegularExpression::UseUnicodePropertiesOption;
    if ( case_insensitive )
        opt |= QRegularExpression::CaseInsensitiveOption;
    include_.setPatternOptions( opt );
    exclude_.setPatternOptions( opt );
    include_.setPattern( includeText_ );
    exclude_.setPattern( excludeText_ );
}

bool RegExpFilter::hasMatch( QString str ) const
{
    return include_.match( str ).hasMatch()
           && ( exclude_.pattern() == "" || !exclude_.match( str ).hasMatch() );
}

QString RegExpFilter::errorMessage() const
{
    QString res = regExpErrorMsg( "include", include_ );
    if ( !res.isEmpty() )
        res += ", ";
    res += regExpErrorMsg( "exclude", exclude_ );
    return res;
}

QString RegExpFilter::regExpErrorMsg( QString name,
                                      const QRegularExpression &regExp )
{
    if ( regExp.isValid() )
        return "";
    QString res = name;
    auto offset = regExp.patternErrorOffset();
    if ( offset != -1 ) {
        res += "[" + QString::number( offset ) + "]";
    }
    res += ":";
    res += regExp.errorString();
    return res;
}

RegExpFilter &RegExpFilter::operator=( const RegExpFilter &other )
{
    include_ = other.include_;
    exclude_ = other.exclude_;
    return *this;
}