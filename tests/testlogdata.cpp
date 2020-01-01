/*
 * Copyright (C) 2009, 2010 Nicolas Bonnefon and other contributors
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

#include <QSignalSpy>
#include <QMutexLocker>
#include <QFile>

#include "testlogdata.h"
#include "logdata.h"

#if !defined( TMPDIR )
#define TMPDIR "/tmp"
#endif

static const qint64 VBL_NB_LINES = 4999999LL;
static const int VBL_LINE_PER_PAGE = 70;
static const char* vbl_format="LOGDATA is a part of glogg, we are going to test it thoroughly, this is line\t\t%07d\n";
static const int VBL_LINE_LENGTH = (76+2+7) ; // Without the final '\n' !
static const int VBL_VISIBLE_LINE_LENGTH = (76+8+4+7); // Without the final '\n' !

static const qint64 SL_NB_LINES = 5000LL;
static const int SL_LINE_PER_PAGE = 70;
static const char* sl_format="LOGDATA is a part of glogg, we are going to test it thoroughly, this is line %06d\n";
static const int SL_LINE_LENGTH = 83; // Without the final '\n' !

static const char* partial_line_begin = "123... beginning of line.";
static const char* partial_line_end = " end of line 123.\n";

void TestLogData::initTestCase()
{
    QVERIFY( generateDataFiles() );
}

void TestLogData::simpleLoad()
{
    LogData logData;
    QSignalSpy progressSpy( &logData, SIGNAL( loadingProgressed( int ) ) );

    // Register for notification file is loaded
    CONNECT(&logData, loadingFinished, this, loadingFinished);

    QBENCHMARK {
        logData.attachFile( TMPDIR "/verybiglog.txt" );
        // Wait for the loading to be done
        {
            QApplication::exec();
        }
    }

    // Disconnect all signals
    disconnect( &logData, 0 );
}

void TestLogData::multipleLoad()
{
    LogData logData;
    QSignalSpy finishedSpy( &logData, SIGNAL( loadingFinished( bool ) ) );

    // Register for notification file is loaded
    CONNECT(&logData, loadingFinished, this, loadingFinished);

    // Start loading the VBL
    logData.attachFile( TMPDIR "/verybiglog.txt" );

    // Immediately interrupt the loading
    logData.interruptLoading();

    // and wait for the signal
    QApplication::exec();

    // Check we have an empty file
    QCOMPARE( finishedSpy.count(), 1 );
    // TODO: check loadingFinished arg == false
    QCOMPARE( logData.getNbLine(), 0LL );
    QCOMPARE( logData.getMaxLength(), 0 );
    QCOMPARE( logData.getFileSize(), 0LL );

    // Restart the VBL
    logData.attachFile( TMPDIR "/verybiglog.txt" );

    // Ensure the counting has started
    {
        QMutex mutex;
        QWaitCondition sleep;
        // sleep.wait( &mutex, 10 );
    }

    // Load the SL (should block until VBL is fully indexed)
    logData.attachFile( TMPDIR "/smalllog.txt" );

    // and wait for the 2 signals (one for each file)
    QApplication::exec();
    QApplication::exec();

    // Check we have the small log loaded
    QCOMPARE( finishedSpy.count(), 3 );
    QCOMPARE( logData.getNbLine(), SL_NB_LINES );
    QCOMPARE( logData.getMaxLength(), SL_LINE_LENGTH );
    QCOMPARE( logData.getFileSize(), SL_NB_LINES * (SL_LINE_LENGTH+1LL) );

    // Restart the VBL again
    logData.attachFile( TMPDIR "/verybiglog.txt" );

    // Immediately interrupt the loading
    logData.interruptLoading();

    // and wait for the signal
    QApplication::exec();

    // Check the small log has been restored
    QCOMPARE( finishedSpy.count(), 4 );
    QCOMPARE( logData.getNbLine(), SL_NB_LINES );
    QCOMPARE( logData.getMaxLength(), SL_LINE_LENGTH );
    QCOMPARE( logData.getFileSize(), SL_NB_LINES * (SL_LINE_LENGTH+1LL) );

    // Disconnect all signals
    disconnect( &logData, 0 );
}

//
// Private functions
//
void TestLogData::loadingFinished()
{
    QApplication::quit();
}

bool TestLogData::generateDataFiles()
{
    char newLine[90];

    QFile file( TMPDIR "/verybiglog.txt" );
    if ( file.open( QIODevice::WriteOnly ) ) {
        for (int i = 0; i < VBL_NB_LINES; i++) {
            snprintf(newLine, 89, vbl_format, i);
            file.write( newLine, qstrlen(newLine) );
        }
    }
    else {
        return false;
    }
    file.close();

    file.setFileName( TMPDIR "/smalllog.txt" );
    if ( file.open( QIODevice::WriteOnly ) ) {
        for (int i = 0; i < SL_NB_LINES; i++) {
            snprintf(newLine, 89, sl_format, i);
            file.write( newLine, qstrlen(newLine) );
        }
    }
    else {
        return false;
    }
    file.close();

    return true;
}
