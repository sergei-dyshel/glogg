/*
 * Copyright (C) 2013, 2014 Nicolas Bonnefon and other contributors
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

#include "abstractlogview.h"
#include "log.h"

#include "persistentinfo.h"
#include "configuration.h"
#include "quickfindmux.h"

#include "qfnotifications.h"
#include "signal_slot.h"
#include "abstractlogview.h"

ENUM_DEFINE(QFDirection, "QFDirection",
            ({{QFDirection::Forward, "Forward"},
              {QFDirection::Backward, "Backward"}}))

QuickFindMux::QuickFindMux( std::shared_ptr<QuickFindPattern> pattern ) :
    QObject(), pattern_( pattern ), registeredSearchables_()
{
    selector_ = nullptr;

    // Forward the pattern's signal to our listeners
    CONNECT(pattern_.get(), patternUpdated, this, notifyPatternChanged);
}

//
// Public member functions
//
void QuickFindMux::registerSelector(
        const QuickFindMuxSelectorInterface* selector )
{
    LOG(logDEBUG) << "QuickFindMux::registerSelector";

    // The selector object we will use when forwarding search requests
    selector_ = selector;

    unregisterAllSearchables();

    if ( selector ) {
        for ( auto i: selector_->getAllSearchables() )
            registerSearchable( i );
    }
    else {
        // null selector, all is well, we don't do anything.
    }
}

void QuickFindMux::setDirection( QFDirection direction )
{
    LOG(logDEBUG) << "QuickFindMux::setDirection: new direction: " << direction;
    currentDirection_ = direction;
}

//
// Public slots
//
void QuickFindMux::searchNext()
{
    LOG(logDEBUG) << "QuickFindMux::searchNext";
    if ( currentDirection_ == QFDirection::Forward )
        searchForward();
    else
        searchBackward();
}

void QuickFindMux::searchPrevious()
{
    LOG(logDEBUG) << "QuickFindMux::searchPrevious";
    if ( currentDirection_ == QFDirection::Forward )
        searchBackward();
    else
        searchForward();
}

void QuickFindMux::searchForward()
{
    LOG(logDEBUG) << "QuickFindMux::searchForward";

    if ( auto searchable = getSearchableWidget() )
        searchable->searchForward();
}

void QuickFindMux::searchBackward()
{
    LOG(logDEBUG) << "QuickFindMux::searchBackward";

    if ( auto searchable = getSearchableWidget() )
        searchable->searchBackward();
}

void QuickFindMux::setNewPattern(
        const QString& new_pattern, bool ignore_case )
{
    static std::shared_ptr<Configuration> config =
        Persistent<Configuration>( "settings" );

    LOG(logDEBUG) << "QuickFindMux::setNewPattern";

    // If we must do an incremental search, we do it now
    if ( config->isQuickfindIncremental() ) {
        pattern_->changeSearchPattern( new_pattern, ignore_case );
        if ( auto searchable = getSearchableWidget() ) {
            if ( currentDirection_ == QFDirection::Forward )
                searchable->incrementallySearchForward();
            else
                searchable->incrementallySearchBackward();
        }
    }
}

void QuickFindMux::confirmPattern(
        const QString& new_pattern, bool ignore_case )
{
    static std::shared_ptr<Configuration> config =
        Persistent<Configuration>( "settings" );

    pattern_->changeSearchPattern( new_pattern, ignore_case );

    // if non-incremental, we perform the search now
    if ( ! config->isQuickfindIncremental() ) {
        searchNext();
    }
    else {
        if ( auto searchable = getSearchableWidget() )
            searchable->incrementalSearchStop();
    }
}

void QuickFindMux::cancelSearch()
{
    static std::shared_ptr<Configuration> config =
        Persistent<Configuration>( "settings" );

    if ( config->isQuickfindIncremental() ) {
        if ( auto searchable = getSearchableWidget() )
            searchable->incrementalSearchAbort();
    }
    else {
        changeQuickFind( "", QFDirection::Forward );
    }
}

//
// Private slots
//
void QuickFindMux::changeQuickFind(
        const QString& new_pattern, QFDirection new_direction )
{
    pattern_->changeSearchPattern( new_pattern );
    setDirection( new_direction );
}

void QuickFindMux::notifyPatternChanged()
{
    emit patternChanged( pattern_->getPattern() );
}

//
// Private member functions
//

// Use the registered 'selector' to determine where to send the search requests.
SearchableWidgetInterface* QuickFindMux::getSearchableWidget() const
{
    LOG(logDEBUG) << "QuickFindMux::getSearchableWidget";

    SearchableWidgetInterface* searchable = nullptr;

    if ( selector_ )
        searchable = selector_->getActiveSearchable();
    else
        LOG(logWARNING) << "QuickFindMux::getActiveSearchable() no registered selector";

    return searchable;
}

void QuickFindMux::registerSearchable( AbstractLogView* searchable )
{
    LOG(logDEBUG) << "QuickFindMux::registerSearchable";

    // The searchable can change our qf pattern
    CONNECT(searchable, changeQuickFind, this, changeQuickFind);
    // Send us notifications
    CONNECT(searchable, notifyQuickFind, this, notify);

    // And clear them
    CONNECT(searchable, clearQuickFindNotification, this, clearNotification);
    // Search can be initiated by the view itself
    CONNECT(searchable, searchNext, this, searchNext);
    CONNECT(searchable, searchPrevious, this, searchPrevious);

    registeredSearchables_.push_back( searchable );
}

void QuickFindMux::unregisterAllSearchables()
{
    for ( auto searchable: registeredSearchables_ )
        disconnect( searchable, 0, this, 0 );

    registeredSearchables_.clear();
}
