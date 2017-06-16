/*
 * Copyright (C) 2010 Nicolas Bonnefon and other contributors
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

// This file implements QuickFindPattern.
// This class implements part of the Quick Find mechanism, it only stores the
// current search pattern, once it has been confirmed (return pressed),
// it can be asked to return the matches in a specific string.

#include "quickfindpattern.h"

#include "persistentinfo.h"
#include "configuration.h"

QuickFindPattern::QuickFindPattern() : QObject(), regexp_()
{
    active_ = false;
}

void QuickFindPattern::changeSearchPattern( const QString& pattern )
{
    regexp_.setPattern( pattern );

    if ( regexp_.isValid() && ( ! regexp_.pattern().isEmpty() ) )
        active_ = true;
    else
        active_ = false;

    emit patternUpdated();
}

void QuickFindPattern::changeSearchPattern( const QString& pattern, bool ignoreCase )
{
  regexp_.setPatternOptions(ignoreCase
                                ? QRegularExpression::CaseInsensitiveOption
                                : QRegularExpression::NoPatternOption);
  changeSearchPattern(pattern);
}

bool QuickFindPattern::matchLine( const QString& line,
        QList<QuickFindMatch>& matches ) const
{
    matches.clear();

    if ( active_ ) {
        int pos = 0;
        while ( true ) {
            auto match = regexp_.match(line, pos);
            if (!match.hasMatch())
                break;
            pos = match.capturedStart();
            int length = match.capturedLength();
            matches << QuickFindMatch( pos, length );
            pos += length;
        }
    }

    return ( matches.count() > 0 );
}

bool QuickFindPattern::isLineMatching( const QString& line, int column ) const
{
    if ( ! active_ )
        return false;
    auto match = regexp_.match( line, column );
    if ( match.hasMatch() ) {
        lastMatchStart_ = match.capturedStart();
        lastMatchEnd_   = match.capturedEnd() - 1;

        return true;
    }
    else
        return false;
}

bool QuickFindPattern::isLineMatchingBackward(
        const QString& line, int column ) const
{
    int pos = 0;

    if ( ! active_ )
        return false;
    QRegularExpressionMatch match;
    if ( ( pos = line.lastIndexOf( regexp_, column, &match ) ) != -1 ) {
        lastMatchStart_ = pos;
        lastMatchEnd_   = match.capturedEnd() - 1;

        return true;
    }
    else
        return false;
}

void QuickFindPattern::getLastMatch( int* start_col, int* end_col ) const
{
    *start_col = lastMatchStart_;
    *end_col   = lastMatchEnd_;
}
