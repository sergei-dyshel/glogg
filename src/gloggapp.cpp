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
#include <QStyleFactory>

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
    CONNECT(&window, setAppStyle, this, setAppStyle);
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

void GloggApp::setDarkPalette()
{
    // taken from https://gist.github.com/QuantumCD/6245215
    QColor darkGray(53, 53, 53);
    QColor gray(128, 128, 128);
    QColor black(25, 25, 25);
    QColor blue(42, 130, 218);

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, darkGray);
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, black);
    darkPalette.setColor(QPalette::AlternateBase, darkGray);
    darkPalette.setColor(QPalette::ToolTipBase, blue);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, darkGray);
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Link, blue);
    darkPalette.setColor(QPalette::Highlight, blue);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    darkPalette.setColor(QPalette::Active, QPalette::Button, gray.darker());
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, gray);
    darkPalette.setColor(QPalette::Disabled, QPalette::Light, darkGray);
    setPalette(darkPalette);
}

void GloggApp::setAppStyle(const QString &styleName, const QString &stylesheet)
{
    auto style = QStyleFactory::create(styleName);
    setStyle(style);
    setStyleSheet(stylesheet);
    if (stylesheet == "") {
        setPalette(style->standardPalette());
    } else {
        setDarkPalette();
    }
}
