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

#pragma once

#include "fwd.h"

#include <QTabBar>

class TabBar : public QTabBar {
    Q_OBJECT

  public:
    explicit TabBar(QWidget *parent);

    bool sameTabBar(const TabInfo &tab) const;
    void insertDropTab(int index, const TabInfo &tab);
    int removeDropTab();
    void setPinned(int index, bool pinned);
    bool isPinned(int index) const;

  signals:
    void dragStarted(int tabIndex);
    void tabCloseRequested(int index);
    void popupMenuRequested(const QPoint &pos, int tabIndex);
    void dragAndDrop(int dropTabIndex, const TabInfo &tab);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *mouseEvent) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private:
    bool shouldShowDropTab(int index, const TabInfo &tab);

    QPoint dragStartPos_;
    int acceptTabIndex_ = -1;
    int dropTabIndex_ = -1;
};