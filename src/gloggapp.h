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

#ifndef GLOGGAPP_H
#define GLOGGAPP_H

#include "externalcom.h"
#include "mainwindow.h"
#include "session.h"

#include <memory>
#include <list>

#include <QApplication>

// Subclass QApplication to add a custom event handler
class GloggApp : public QApplication
{
    Q_OBJECT
  public:
    GloggApp(int &argc, char **argv) : QApplication(argc, argv) {}

    MainWindow &newWindow();

  public slots:
    void loadFileInActiveWindow(const QString &filename);

  signals:
    void loadFile( const QString& file_name );
    void settingsChanged();

  private slots:
    void setAppStyle(const QString &style, const QString &stylesheet);

  protected:
#ifdef __APPLE__
    virtual bool event( QEvent* event );
#endif

  private:
    void onWindowActivated(MainWindow &window);
    void setDarkPalette();

    std::shared_ptr<Session> session_;
    std::list<MainWindow> windows_;
    MainWindow *lastActiveWindow_ = nullptr;
};

#endif
