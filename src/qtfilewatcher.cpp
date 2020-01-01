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

#include "log.h"

#include "qtfilewatcher.h"

#include <QStringList>
#include <QFileInfo>

QtFileWatcher::QtFileWatcher() : FileWatcher(), qtFileWatcher_( this )
{
    monitoringState_ = None;

    CONNECT(&qtFileWatcher_, fileChanged, this, fileChangedOnDisk);

    CONNECT(&qtFileWatcher_, directoryChanged, this, directoryChangedOnDisk);
}

QtFileWatcher::~QtFileWatcher()
{
    disconnect( &qtFileWatcher_ );
}

void QtFileWatcher::addFile( const QString& fileName )
{
    LOG(logDEBUG) << "QtFileWatcher::addFile " << fileName.toStdString();

    QFileInfo fileInfo = QFileInfo( fileName );

    if ( fileMonitored_.isEmpty() ) {
        fileMonitored_ = fileName;

        // Initialise the Qt file watcher
        qtFileWatcher_.addPath( fileInfo.path() );

        if ( fileInfo.exists() ) {
            LOG(logDEBUG) << "QtFileWatcher::addFile: file exists.";
            qtFileWatcher_.addPath( fileName );
            monitoringState_ = FileExists;
        }
        else {
            LOG(logDEBUG) << "QtFileWatcher::addFile: file doesn't exist.";
            monitoringState_ = FileRemoved;
        }
    }
    else {
        LOG(logWARNING) << "QtFileWatcher::addFile " << fileName.toStdString()
            << "- Already watching a file (" << fileMonitored_.toStdString()
            << ")!";
    }
}

void QtFileWatcher::removeFile( const QString& fileName )
{
    LOG(logDEBUG) << "QtFileWatcher::removeFile " << fileName.toStdString();

    QFileInfo fileInfo = QFileInfo( fileName );

    if ( fileName == fileMonitored_ ) {
        if ( monitoringState_ == FileExists )
            qtFileWatcher_.removePath( fileName );
        qtFileWatcher_.removePath( fileInfo.path() );
        fileMonitored_.clear();
        monitoringState_ = None;
    }
    else {
        LOG(logWARNING) << "QtFileWatcher::removeFile - The file is not watched!";
    }

    // For debug purpose:
    foreach (QString str, qtFileWatcher_.files()) {
        LOG(logERROR) << "File still watched: " << str.toStdString();
    }
    foreach (QString str, qtFileWatcher_.directories()) {
        LOG(logERROR) << "Directories still watched: " << str.toStdString();
    }
}

//
// Slots
//

void QtFileWatcher::fileChangedOnDisk( const QString& filename )
{
    LOG(logDEBUG) << "QtFileWatcher::fileChangedOnDisk " << filename.toStdString();

    if ( ( monitoringState_ == FileExists ) && ( filename == fileMonitored_ ) )
    {
        emit fileChanged( filename );

        // If the file has been removed...
        if ( !QFileInfo( filename ).exists() )
            monitoringState_ = FileRemoved;
    }
    else
        LOG(logWARNING) << "QtFileWatcher::fileChangedOnDisk - call from Qt but no file monitored";
}

void QtFileWatcher::directoryChangedOnDisk( const QString& filename )
{
    LOG(logDEBUG) << "QtFileWatcher::directoryChangedOnDisk " << filename.toStdString();

    if ( monitoringState_ == FileRemoved ) {
        if ( QFileInfo( fileMonitored_ ).exists() ) {
            LOG(logDEBUG) << "QtFileWatcher::directoryChangedOnDisk - our file reappeared!";

            // The file has been recreated, we have to watch it again.
            monitoringState_ = FileExists;

            // Restore the Qt file watcher (automatically cancelled
            // when the file is deleted)
            qtFileWatcher_.addPath( fileMonitored_ );

            emit fileChanged( fileMonitored_ );
        }
        else {
            LOG(logWARNING) << "QtFileWatcher::directoryChangedOnDisk - not the file we are watching";
        }
    }
    else if ( monitoringState_ == FileExists )
    {
        if ( ! QFileInfo( fileMonitored_ ).exists() ) {
            LOG(logDEBUG) << "QtFileWatcher::directoryChangedOnDisk - our file disappeared!";

            monitoringState_ = FileRemoved;

            emit fileChanged( filename );
        }
        else {
            LOG(logWARNING) << "QtFileWatcher::directoryChangedOnDisk - not the file we are watching";
        }
    }

}
