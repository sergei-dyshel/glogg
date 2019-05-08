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

#include "tab_bar.h"

#include "exception.h"
#include "log.h"
#include "tab_info.h"

#include <QApplication>
#include <QMimeData>
#include <QMouseEvent>

TabBar::TabBar(QWidget *parent) : QTabBar(parent) { setAcceptDrops(true); }

void TabBar::mousePressEvent(QMouseEvent *event)
{
    int tabIndex = tabAt(event->pos());
    DEBUG_THIS << tabIndex;
    if (event->button() == Qt::LeftButton && tabIndex != -1) {
        setCurrentIndex(tabIndex);
        dragStartPos_ = event->pos();
    }
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    int tabIndex = tabAt(event->pos());
    DEBUG_THIS << tabIndex;

    if (tabIndex == -1)
        return;

    event->ignore();
    if (event->button() == Qt::RightButton)
        emit popupMenuRequested(event->pos(), tabIndex);
}

void TabBar::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - dragStartPos_).manhattanLength()
         < QApplication::startDragDistance())
        return;

    int tabIndex = tabAt(dragStartPos_);
    if (tabIndex < 0)
        return;

    emit dragStarted(tabIndex);
}

void TabBar::insertDropTab(int index, const TabInfo &tab)
{
    ASSERT(0 <= index && index <= count());
    insertTab(index, "");
    tabButton(index, LeftSide)->deleteLater();
    setTabButton(index, LeftSide, nullptr);
    setTabText(index, tab.tabText);
    setTabEnabled(index, false);
    dropTabIndex_ = index;
    DEBUG_THIS << "Inserting drop tab at" << LOG_EXPR(dropTabIndex_);
    acceptTabIndex_ = -1;
}

bool TabBar::sameTabBar(const TabInfo &tab) const
{
    return (tab.pid == QApplication::applicationPid()
            && tab.tabBar == reinterpret_cast<qint64>(this));
}

bool TabBar::shouldShowDropTab(int index, const TabInfo &tab)
{
    if (!sameTabBar(tab))
        return true;

    if (index == tab.tabIndex) {
        DEBUG_THIS << "Can not drop before dragged tab";
        return false;
    }
    if (index == tab.tabIndex + 1) {
        DEBUG_THIS << "Can not drop after dragged tab";
        return false;
    }
    if (index == -1 && (tab.tabIndex == count() - 1)) {
        DEBUG_THIS << "Can not drop after dragged tab because it's the last";
        return false;
    }
    return true;
}

void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    TabInfo tab;
    if (!TabInfo::tryDecodeMimeData(event->mimeData(), &tab))
        return;

    event->setDropAction((event->keyboardModifiers() & Qt::ControlModifier)
                             ? Qt::CopyAction
                             : Qt::MoveAction);
    int tabIndex = tabAt(event->pos());
    DEBUG_THIS << LOG_EXPR(tabIndex);
    if (tabIndex == -1)
        tabIndex = count();
    if (!shouldShowDropTab(tabIndex, tab)) {
        event->accept(tabRect(tabIndex));
        return;
    }
    ASSERT(dropTabIndex_ == -1);
    dropTabIndex_ = tabIndex;
    insertDropTab(dropTabIndex_, tab);
    event->accept();
    DEBUG_THIS << LOG_EXPR(dropTabIndex_);
}

void TabBar::dragMoveEvent(QDragMoveEvent *event)
{
    TabInfo tab;
    bool decoded = TabInfo::tryDecodeMimeData(event->mimeData(), &tab);
    ASSERT(decoded);
    event->setDropAction((event->keyboardModifiers() & Qt::ControlModifier)
                             ? Qt::CopyAction
                             : Qt::MoveAction);
    int tabIndex = tabAt(event->pos());
    if (tabIndex == -1)
        tabIndex = count();
    if (tabIndex == acceptTabIndex_) {
        event->accept();
        return;
    }
    DEBUG_THIS << LOG_EXPR(dropTabIndex_) << LOG_EXPR(tabIndex);
    if (dropTabIndex_ == -1) {
        if (!shouldShowDropTab(tabIndex, tab)) {
            acceptTabIndex_ = tabIndex;
            event->accept(tabRect(tabIndex));
            return;
        }
    } else {
        if (tabIndex == dropTabIndex_ || (tabIndex == dropTabIndex_ + 1)) {
            acceptTabIndex_ = tabIndex;
            event->accept(tabRect(tabIndex));
            return;
        }
        if (dropTabIndex_ < tabIndex)
            --tabIndex;
        removeDropTab();
        if (!shouldShowDropTab(tabIndex, tab))
            return;
    }
    insertDropTab(tabIndex, tab);
    event->accept();
}

void TabBar::dragLeaveEvent(QDragLeaveEvent *)
{
    if (dropTabIndex_ == -1)
        return;
    removeDropTab();
}

void TabBar::dropEvent(QDropEvent *event)
{
    TabInfo tab;
    bool tabDecoded = TabInfo::tryDecodeMimeData(event->mimeData(), &tab);
    ASSERT(tabDecoded);
    INFO << tab;
    if (dropTabIndex_ == -1)
        return;
    event->acceptProposedAction();
    int dropTabIndex = removeDropTab();
    if (sameTabBar(tab)) {
        DEBUG_THIS << "Moving within same tabBar" << LOG_EXPR(tab.tabIndex)
              << LOG_EXPR(dropTabIndex_);
        moveTab(tab.tabIndex,
                tab.tabIndex < dropTabIndex ? dropTabIndex - 1 : dropTabIndex);
    } else {
        emit dragAndDrop(dropTabIndex, tab);
    }
}

int TabBar::removeDropTab()
{
    ASSERT(dropTabIndex_ != -11);
    DEBUG_THIS << "Removing drop tab" << dropTabIndex_;
    removeTab(dropTabIndex_);
    int res = dropTabIndex_;
    dropTabIndex_= -1;
    return res;
}

void TabBar::setPinned(int index, bool pinned)
{
    tabButton(index, QTabBar::LeftSide)->setVisible(!pinned);
}

bool TabBar::isPinned(int index) const
{
    return !tabButton(index, QTabBar::LeftSide)->isVisible();
}