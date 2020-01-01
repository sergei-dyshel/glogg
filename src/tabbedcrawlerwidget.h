/*
 * Copyright (C) 2014 Nicolas Bonnefon and other contributors
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

#ifndef TABBEDCRAWLERWIDGET_H
#define TABBEDCRAWLERWIDGET_H

#include <QTabWidget>
#include <QTabBar>

#include "loadingstatus.h"
#include "tab_bar.h"

class CrawlerWidget;

// This class represents glogg's main widget, a tabbed
// group of CrawlerWidgets.
// This is a very slightly customised QTabWidget, with
// a particular style.
class TabbedCrawlerWidget : public QTabWidget
{
  Q_OBJECT
    public:
      TabbedCrawlerWidget(QWidget *parent);
      virtual ~TabbedCrawlerWidget() {}

      // "Overridden" insertTab/removeTab that automatically
      // show/hide the tab bar
      // The tab is created with the 'old data' icon.
      int insertTab(int index, CrawlerWidget *crawler, const QString &label,
                    bool setCurrent = false);
      void removeTab( int index );

      // Set the data status (icon) for the tab number 'index'
      void setTabDataStatus( int index, DataStatus status );

      TabBar &tabBar() { return myTabBar_; }

    signals:
      void openInAnotherServer(int tab, QString server);
      void dragAndDrop(int dropTabIndex, const TabInfo &tab);
      void duplicateTab(int tabIndex);
      void crawlerAdded(CrawlerWidget *crawler);

    protected:
      void keyPressEvent( QKeyEvent* event ) override;
      void dragEnterEvent(QDragEnterEvent *event) override;
      void dragLeaveEvent(QDragLeaveEvent *event) override;
      void dropEvent(QDropEvent *event) override;

    private:
      void showTabPopupMenu(const QPoint &pos, int tabindex);
      void onTabDragStarted(int tabIndex);
      void closeTabs(int begin, int end);

      const QIcon olddata_icon_;
      const QIcon newdata_icon_;
      const QIcon newfiltered_icon_;

      TabBar myTabBar_;
      bool allowDrops_ = true;
};

#endif
