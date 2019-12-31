/*
 * Copyright (C) 2017 Nicolas Bonnefon and other contributors
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

#include "gloggapp.h"
#include "signal_slot.h"

#include <QFileOpenEvent>

#ifdef __APPLE__
bool GloggApp::event( QEvent *event )
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        emit loadFile( openEvent->file() );
    }

    return QApplication::event(event);
}
#endif

MainWindow &GloggApp::newWindow()
{
    windows_.emplace_front(Session::the);
    auto &window = windows_.front();
    lastActiveWindow_ = &window;
    DEBUG << "Window" << &window << "created";
    connect(&window, &MainWindow::newWindow, [=]() { newWindow().show(); });
    connect(&window, &MainWindow::windowActivated,
            [=, &window]() { onWindowActivated(window); });
    connect(&window, &MainWindow::exitRequested, this, &QCoreApplication::quit);
    connect(&window, &MainWindow::setAppStyleSheet,
            [=](const QString &styleSheet) { setStyleSheet(styleSheet); });
    connect(&window, &MainWindow::optionsChanged, this,
            &GloggApp::settingsChanged);
    connect(this, &GloggApp::settingsChanged, &window,
            &MainWindow::applySettings);
    CONNECT(this, loadFile, &window, loadFileNonInteractive);
    window.updateCurrentStyle(true /* trigger */);
    return window;
}

void GloggApp::onWindowActivated(MainWindow &window)
{
    DEBUG << "Window" << &window << "activated";
    lastActiveWindow_ = &window;
}

void GloggApp::loadFileInActiveWindow(const QString &filename)
{
    lastActiveWindow_->loadFileNonInteractive(filename);
}
