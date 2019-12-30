/*
 * Copyright (C) 2014, 2015 Nicolas Bonnefon and other contributors
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

#include "tabbedcrawlerwidget.h"

#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QInputDialog>
#include <QWidgetAction>
#include <QLabel>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>

#include "crawlerwidget.h"
#include "externalcom.h"
#include "tab_info.h"

#include "log.h"
#include "qt_utils.h"

TabbedCrawlerWidget::TabbedCrawlerWidget(QWidget *parent) : QTabWidget(parent),
    olddata_icon_( loadPngAndAdjustColor(":/images/olddata_icon.png") ),
    newdata_icon_( loadPngAndAdjustColor(":/images/newdata_icon.png") ),
    newfiltered_icon_( ":/images/newfiltered_icon.png" ),
    myTabBar_(this)
{
#ifdef WIN32
    myTabBar_.setStyleSheet( "QTabBar::tab {\
            height: 20px; "
            "} "
            "QTabBar::close-button {\
              height: 6px; width: 6px;\
              subcontrol-origin: padding;\
              subcontrol-position: left;\
             }" );
#else
    myTabBar_.setStyleSheet(
            "QTabBar::tab {"
            "} "
            "QTabBar::close-button {\
              subcontrol-origin: padding;\
              subcontrol-position: left;\
             }" );
#endif
    setTabBar( &myTabBar_ );
    disableTabFocus(this);
    setAcceptDrops(true);
    disableTabFocusOnChildren<QToolButton>(&myTabBar_);

    connect(&myTabBar_, &QTabBar::tabCloseRequested, this,
            &TabbedCrawlerWidget::tabCloseRequested);
    connect(&myTabBar_, &TabBar::popupMenuRequested, this,
            &TabbedCrawlerWidget::showTabPopupMenu);
    connect(&myTabBar_, &TabBar::dragStarted, this,
            &TabbedCrawlerWidget::onTabDragStarted);
    connect(&myTabBar_, &TabBar::dragAndDrop, this,
            &TabbedCrawlerWidget::dragAndDrop);
}

// I know hiding non-virtual functions from the base class is bad form
// and I do it here out of pure laziness: I don't want to encapsulate
// QTabBar with all signals and all just to implement this very simple logic.
// Maybe one day that should be done better...

int TabbedCrawlerWidget::insertTab(int tabIndex, QWidget* page,
                                   const QString& label, bool setCurrent)
{
    int index = QTabWidget::insertTab(tabIndex, page, label );

    if ( auto crawler = dynamic_cast<CrawlerWidget*>( page ) ) {
        // Mmmmhhhh... new Qt5 signal syntax create tight coupling between
        // us and the sender, baaaaad....

        // Listen for a changing data status:
        disconnect(crawler, &CrawlerWidget::dataStatusChanged, 0, 0);
        connect( crawler, &CrawlerWidget::dataStatusChanged,
                [ this, index ]( DataStatus status ) { setTabDataStatus( index, status ); } );
    }

    // Display the icon
    QLabel* icon_label = new QLabel();
    icon_label->setPixmap( olddata_icon_.pixmap( 11, 12 ) );
    icon_label->setAlignment( Qt::AlignCenter );
    myTabBar_.setTabButton( index, QTabBar::RightSide, icon_label );
    auto *closeButton = new QToolButton();
    closeButton->setIcon(loadSvgAndAdjustColor(":images/close.svg"));
    closeButton->setIconSize(QSize(11, 12));
    connect(closeButton, &QToolButton::clicked, [=] {
        emit tabCloseRequested(index);
    });
    myTabBar_.setTabButton( index, QTabBar::LeftSide, closeButton);

    LOG(logDEBUG) << "insertTab, count = " << count();
    LOG(logDEBUG) << "width = " << olddata_icon_.pixmap( 11, 12 ).devicePixelRatio();

    if (setCurrent)
        setCurrentIndex(index);
    allowDrops_ = false;
    return index;
}

void TabbedCrawlerWidget::removeTab( int index )
{
    QTabWidget::removeTab( index );

    if (count() == 0)
        allowDrops_ = true;
}

void TabbedCrawlerWidget::onTabDragStarted(int tabIndex)
{
    auto drag = new QDrag(this);
    TabInfo tab;
    tab.pid = QApplication::applicationPid();
    tab.tabBar = reinterpret_cast<qint64>(&myTabBar_);
    tab.tabText = myTabBar_.tabText(tabIndex);
    tab.tabIndex = tabIndex;
    auto crawler = dynamic_cast<CrawlerWidget*>(widget(tabIndex));
    tab.filename = crawler->logData()->attachedFilename();
    INFO << "Starting tab drag" << tab;

    drag->setMimeData(tab.toMimeData());
    drag->setPixmap(myTabBar_.grab(myTabBar_.tabRect(tabIndex)));
    drag->setHotSpot(QPoint(0, 0));

    auto dropAction
        = drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::MoveAction);
    INFO << "dropAction" << dropAction;
}

void TabbedCrawlerWidget::closeTabs(int begin, int end)
{
    int index = begin;
    for (int i = begin; i < end; ++i)
        if (!myTabBar_.isPinned(index))
            emit tabCloseRequested(index);
        else
            ++index;
}

void TabbedCrawlerWidget::showTabPopupMenu(const QPoint &pos, int tab)
{
    QMenu menu(this);

    menu.addAction("Close", [=]() { emit tabCloseRequested(tab); });

    if (myTabBar_.isPinned(tab))
        menu.addAction("Unpin", [=]() { myTabBar_.setPinned(tab, false); });
    else
        menu.addAction("Pin", [=]() { myTabBar_.setPinned(tab, true); });

    menu.addAction("Rename", [=]() {
        auto old_name = myTabBar_.tabText(tab);
        bool ok;
        auto new_name
            = QInputDialog::getText( this, "Rename tab", "Enter new name",
                                        QLineEdit::Normal, old_name, &ok );
        if (ok)
            myTabBar_.setTabText( tab, new_name );
    });

    menu.addAction("Duplicate", [=]() { emit duplicateTab(tab); });

    auto color_submenu = menu.addMenu("Change color...");
    std::list<QColor> colors
        = {Qt::red,      Qt::blue,     Qt::green,       Qt::cyan,
           Qt::magenta,  Qt::yellow,   Qt::darkRed,     Qt::darkGreen,
           Qt::darkBlue, Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow};
    for (auto color : colors) {
        auto action = color_submenu->addAction(
            "", [=]() { myTabBar_.setTabTextColor(tab, color); });
        addColorIconToAction(action, color);
    }

    menu.addAction("Copy path", [=]() {
        QApplication::clipboard()->setText(
            dynamic_cast<CrawlerWidget*>(widget(tab))
                ->logData()
                ->attachedFilename());
    });

    menu.addSeparator();

    menu.addAction("Close all", [=]() {
        closeTabs(0, count()); });
    menu.addAction("Close all before", [=]() {
        closeTabs(0, tab); })
        ->setEnabled(tab > 0);
    menu.addAction("Close all after", [=]() {
        closeTabs(tab + 1, count()); })
        ->setEnabled(tab < count() - 1);

    int numPinned = 0;
    for (int i = 0; i < count(); ++i)
        if (myTabBar_.isPinned(i))
            ++numPinned;

    menu.addAction("Pin all",
                   [=]() {
                       for (int i = 0; i < count(); ++i)
                           myTabBar_.setPinned(i, true);
                   })
        ->setEnabled(numPinned < count());
    menu.addAction("Unpin all",
                   [=]() {
                       for (int i = 0; i < count(); ++i)
                           myTabBar_.setPinned(i, false);
                   })
        ->setEnabled(numPinned > 0);

    menu.addSeparator();
    menu.addAction("Move to start", [=]() { myTabBar_.moveTab(tab, 0); })
        ->setEnabled(tab > 0);
    menu.addAction("Move to end", [=]() { myTabBar_.moveTab(tab, count()); })
        ->setEnabled(tab < count() - 1);

    menu.exec(mapToGlobal(pos));
}

void TabbedCrawlerWidget::keyPressEvent( QKeyEvent* event )
{
    const auto mod = event->modifiers();
    const auto key = event->key();

    LOG(logDEBUG) << "TabbedCrawlerWidget::keyPressEvent";

    // Ctrl + tab
    if ( ( mod == Qt::ControlModifier && key == Qt::Key_Tab ) ||
         ( mod == ( Qt::ControlModifier | Qt::AltModifier | Qt::KeypadModifier ) && key == Qt::Key_Right ) ||
         ( mod == Qt::ControlModifier && key == Qt::Key_PageDown ) ) {
        setCurrentIndex( ( currentIndex() + 1 ) % count() );
    }
    // Ctrl + shift + tab
    else if ( ( mod == ( Qt::ControlModifier | Qt::ShiftModifier ) && key == Qt::Key_Tab ) ||
              ( mod == ( Qt::ControlModifier | Qt::AltModifier | Qt::KeypadModifier ) && key == Qt::Key_Left ) ||
              ( mod == Qt::ControlModifier && key == Qt::Key_PageUp ) ) {
        setCurrentIndex( ( currentIndex() - 1 >= 0 ) ? currentIndex() - 1 : count() - 1 );
    }
    // Ctrl + numbers
    else if ( mod == Qt::ControlModifier && ( key >= Qt::Key_1 && key <= Qt::Key_8 ) ) {
        int new_index = key - Qt::Key_0;
        if ( new_index <= count() )
            setCurrentIndex( new_index - 1 );
    }
    // Ctrl + 9
    else if ( mod == Qt::ControlModifier && key == Qt::Key_9 ) {
        setCurrentIndex( count() - 1 );
    }
    else if ( mod == Qt::ControlModifier && (key == Qt::Key_Q || key == Qt::Key_W) ) {
        emit tabCloseRequested( currentIndex() );
    }
    else {
        QTabWidget::keyPressEvent( event );
    }
}

void TabbedCrawlerWidget::setTabDataStatus( int index, DataStatus status )
{
    LOG(logDEBUG) << "TabbedCrawlerWidget::setTabDataStatus " << index;

    QLabel* icon_label = dynamic_cast<QLabel*>(
            myTabBar_.tabButton( index, QTabBar::RightSide ) );

    if ( icon_label ) {
        const QIcon* icon;
        switch ( status ) {
            case DataStatus::OLD_DATA:
                icon = &olddata_icon_;
                break;
            case DataStatus::NEW_DATA:
                icon = &newdata_icon_;
                break;
            case DataStatus::NEW_FILTERED_DATA:
                icon = &newfiltered_icon_;
                break;
        default:
            return;
        }

        icon_label->setPixmap ( icon->pixmap(12,12) );

    }
}

void TabbedCrawlerWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (!allowDrops_)
        return;
    ASSERT(count() == 0);

    TabInfo tab;
    if (!TabInfo::tryDecodeMimeData(event->mimeData(), &tab))
        return;

    if (myTabBar_.sameTabBar(tab)) {
        DEBUG_THIS << "Can not drop on same tab widget";
        return;
    }
    DEBUG_THIS;
    myTabBar_.insertDropTab(myTabBar_.count(), tab);
    event->accept();
}

void TabbedCrawlerWidget::dragLeaveEvent(QDragLeaveEvent*)
{
    DEBUG_THIS;
    myTabBar_.removeDropTab();
}

void TabbedCrawlerWidget::dropEvent(QDropEvent *event)
{
    DEBUG_THIS;

    TabInfo tab;
    bool tabDecoded = TabInfo::tryDecodeMimeData(event->mimeData(), &tab);
    ASSERT(tabDecoded);

    INFO << tab;
    ASSERT(!myTabBar_.sameTabBar(tab));

    int tabIndex = myTabBar_.removeDropTab();
    emit dragAndDrop(tabIndex, tab);
}
