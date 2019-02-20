/*
 * Copyright (C) 2010, 2013, 2017 Nicolas Bonnefon and other contributors
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

// This file implements QuickFind.
// This class implements the Quick Find mechanism using references
// to the logData, the QFP and the selection passed.
// Search is started just after the selection and the selection is updated
// if a match is found.

#include <QApplication>

#include "log.h"
#include "quickfindpattern.h"
#include "selection.h"
#include "data/abstractlogdata.h"

#include "quickfind.h"

void SearchingNotifier::reset()
{
    dotToDisplay_ = 0;
    startTime_ = QTime::currentTime();
}

void SearchingNotifier::sendNotification( qint64 current_line, qint64 nb_lines )
{
    qint64 progress;
    if ( current_line < 0 )
        progress = ( nb_lines + current_line ) * 100 / nb_lines;
    else
        progress = current_line * 100 / nb_lines;
    emit notify( QFNotificationProgress( progress ) );

    QApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    startTime_ = QTime::currentTime();
}

void QuickFind::LastMatchPosition::set( int line, int column )
{
    if ( ( line_ == -1 ) ||
            ( ( line <= line_ ) && ( column < column_ ) ) )
    {
        line_ = line;
        column_ = column;
    }
}

void QuickFind::LastMatchPosition::set( const FilePosition &position )
{
    set( position.line(), position.column() );
}

bool QuickFind::LastMatchPosition::isLater( int line, int column ) const
{
    if ( line_ == -1 )
        return false;
    else if ( ( line == line_ ) && ( column >= column_ ) )
        return true;
    else if ( line > line_ )
        return true;
    else
        return false;
}

bool QuickFind::LastMatchPosition::isLater( const FilePosition &position ) const
{
    return isLater( position.line(), position.column() );
}

bool QuickFind::LastMatchPosition::isSooner( int line, int column ) const
{
    if ( line_ == -1 )
        return false;
    else if ( ( line == line_ ) && ( column <= column_ ) )
        return true;
    else if ( line < line_ )
        return true;
    else
        return false;
}

bool QuickFind::LastMatchPosition::isSooner( const FilePosition &position ) const
{
    return isSooner( position.line(), position.column() );
}

QuickFind::QuickFind( const AbstractLogData* const logData,
        Selection* selection,
        const QuickFindPattern* const quickFindPattern ) :
    logData_( logData ), selection_( selection ),
    quickFindPattern_( quickFindPattern ),
    lastMatch_(), firstMatch_(), searchingNotifier_(),
    incrementalSearchStatus_()
{
    connect( &searchingNotifier_, SIGNAL( notify( const QFNotification& ) ),
            this, SIGNAL( notify( const QFNotification& ) ) );
}

void QuickFind::incrementalSearchStop()
{
    if ( incrementalSearchStatus_.isOngoing() ) {
        if ( selection_->isEmpty() ) {
            // Nothing found?
            // We reset the selection to what it was
            *selection_ = incrementalSearchStatus_.initialSelection();
        }

        incrementalSearchStatus_ = IncrementalSearchStatus();
    }
}

void QuickFind::incrementalSearchAbort()
{
    if ( incrementalSearchStatus_.isOngoing() ) {
        // We reset the selection to what it was
        *selection_ = incrementalSearchStatus_.initialSelection();
        incrementalSearchStatus_ = IncrementalSearchStatus();
    }
}

RangeInLine QuickFind::incrementallySearchForward()
{
    LOG( logDEBUG ) << "QuickFind::incrementallySearchForward";

    // Position where we start the search from
    FilePosition start_position = selection_->getNextPosition();

    if ( incrementalSearchStatus_.direction() == Forward ) {
        // An incremental search is active, we restart the search
        // from the initial point
        LOG( logDEBUG ) << "Restart search from initial point";
        start_position = incrementalSearchStatus_.position();
    }
    else {
        // It's a new search so we search from the selection
        incrementalSearchStatus_ = IncrementalSearchStatus(
                Forward,
                start_position,
                *selection_ );
    }

    RangeInLine rangeInLine = doSearchForward( start_position );

    if ( rangeInLine ) {
        // We have found a result...
        // ... the caller will jump to this line.
        return rangeInLine;
    }
    else {
        // No result...
        // ... we want the client to show the initial line.
        selection_->clear();
        return RangeInLine(incrementalSearchStatus_.position().line(),
                           Range(0, 1));
    }
}

RangeInLine QuickFind::incrementallySearchBackward()
{
    LOG( logDEBUG ) << "QuickFind::incrementallySearchBackward";

    // Position where we start the search from
    FilePosition start_position = selection_->getPreviousPosition();

    if ( incrementalSearchStatus_.direction() == Backward ) {
        // An incremental search is active, we restart the search
        // from the initial point
        LOG( logDEBUG ) << "Restart search from initial point";
        start_position = incrementalSearchStatus_.position();
    }
    else {
        // It's a new search so we search from the selection
        incrementalSearchStatus_ = IncrementalSearchStatus(
                Backward,
                start_position,
                *selection_ );
    }

    auto rangeInLine = doSearchBackward( start_position );

    if ( rangeInLine ) {
        // We have found a result...
        // ... the caller will jump to this line.
        return rangeInLine;
    }
    else {
        // No result...
        // ... we want the client to show the initial line.
        selection_->clear();
        return RangeInLine(incrementalSearchStatus_.position().line(),
                           Range(0, 1));
    }
}

RangeInLine QuickFind::searchForward()
{
    incrementalSearchStatus_ = IncrementalSearchStatus();

    // Position where we start the search from
    FilePosition start_position = selection_->getNextPosition();

    return doSearchForward( start_position );
}


RangeInLine QuickFind::searchBackward()
{
    incrementalSearchStatus_ = IncrementalSearchStatus();

    // Position where we start the search from
    FilePosition start_position = selection_->getPreviousPosition();

    return doSearchBackward( start_position );
}

// Internal implementation of forward search,
// returns the line where the pattern is found or -1 if not found.
// Parameters are the position the search shall start
RangeInLine QuickFind::doSearchForward( const FilePosition &start_position )
{
    bool found = false;
    int found_start_col;
    int found_end_col;

    if ( searchState_ == SearchState::OnGoing ) {
        // This happens if this function is re-entered via the notifier
        // We want the top most call to restart the search
        searchState_ = SearchState::RestartNeeded;
        return RangeInLine();
    }

    if ( ! quickFindPattern_->isActive() )
        return RangeInLine();

    // Optimisation: if we are already after the last match,
    // we don't do any search at all.
    if ( lastMatch_.isLater( start_position ) ) {
        // Send a notification
        emit notify( QFNotificationReachedEndOfFile() );

        return RangeInLine();
    }

    qint64 line = start_position.line();
    LOG( logDEBUG ) << "Start searching at line " << line;
    // We look at the rest of the first line
    if ( quickFindPattern_->isLineMatching(
                logData_->getExpandedLineString( line ),
                start_position.column() ) ) {
        quickFindPattern_->getLastMatch( &found_start_col, &found_end_col );
        found = true;
    }
    else {
        searchingNotifier_.reset();
        searchState_ = SearchState::OnGoing;
        // And then the rest of the file
        qint64 nb_lines = logData_->getNbLine();
        int start_line = ++line;
        while ( line < nb_lines ) {
            // Check if someone has changed the search within this loop
            if ( searchState_ == SearchState::RestartNeeded ) {
                line = start_line;
                searchState_ = SearchState::OnGoing;
                LOG( logDEBUG ) << "QuickFind restarted at line " << line;
            }

            if ( quickFindPattern_->isLineMatching(
                        logData_->getExpandedLineString( line ) ) ) {
                quickFindPattern_->getLastMatch(
                        &found_start_col, &found_end_col );
                found = true;
                LOG( logDEBUG ) << "QuickFind found!";
                break;
            }
            line++;

            // See if we need to notify of the ongoing search
            // Note this could re-enter this function if the pattern is
            // modified whilst the search is on-going, this is okay.
            searchingNotifier_.ping( line, nb_lines );
        }

        searchState_ = SearchState::Idle;
    }

    if ( found ) {
        selection_->selectPortion(
                line, found_start_col, found_end_col );

        // Clear any notification
        emit clearNotification();

        return RangeInLine(line, Range(found_start_col, found_end_col + 1));
    }
    else {
        // Update the position of the last match
        FilePosition last_match_position = selection_->getPreviousPosition();
        lastMatch_.set( last_match_position );

        // Send a notification
        emit notify( QFNotificationReachedEndOfFile() );

        return RangeInLine();
    }
}

// Internal implementation of backward search,
// returns the line where the pattern is found or -1 if not found.
// Parameters are the position the search shall start
RangeInLine QuickFind::doSearchBackward( const FilePosition &start_position )
{
    bool found = false;
    int start_col;
    int end_col;

    if ( ! quickFindPattern_->isActive() )
        return RangeInLine();

    // Optimisation: if we are already before the first match,
    // we don't do any search at all.
    if ( firstMatch_.isSooner( start_position ) ) {
        // Send a notification
        emit notify( QFNotificationReachedBegininningOfFile() );

        return RangeInLine();
    }

    qint64 line = start_position.line();
    LOG( logDEBUG ) << "Start searching at line " << line;
    // We look at the beginning of the first line
    if (    ( start_position.column() > 0 )
         && ( quickFindPattern_->isLineMatchingBackward(
                 logData_->getExpandedLineString( line ),
                 start_position.column() ) )
       ) {
        quickFindPattern_->getLastMatch( &start_col, &end_col );
        found = true;
    }
    else {
        searchingNotifier_.reset();
        // And then the rest of the file
        qint64 nb_lines = logData_->getNbLine();
        line--;
        while ( line >= 0 ) {
            if ( quickFindPattern_->isLineMatchingBackward(
                        logData_->getExpandedLineString( line ) ) ) {
                quickFindPattern_->getLastMatch( &start_col, &end_col );
                found = true;
                break;
            }
            line--;

            // See if we need to notify of the ongoing search
            searchingNotifier_.ping( -line, nb_lines );
        }
    }

    if ( found )
    {
        selection_->selectPortion( line, start_col, end_col );

        // Clear any notification
        emit clearNotification();

        return RangeInLine(line, Range(start_col, end_col + 1));
    }
    else {
        // Update the position of the first match
        FilePosition first_match_position = selection_->getNextPosition();
        firstMatch_.set( first_match_position );

        // Send a notification
        LOG( logDEBUG ) << "QF: Send BOF notification.";
        emit notify( QFNotificationReachedBegininningOfFile() );

        return RangeInLine();
    }
}

void QuickFind::resetLimits()
{
    lastMatch_.reset();
    firstMatch_.reset();
}

QDebug &operator<<(QDebug &debug, const RangeInLine &rangeInLine)
{
    QDEBUG_COMPAT(debug);
    return debug << "line: " << rangeInLine.line << ", "
                 << "columns: " << rangeInLine.columns;
}
