/*
 * Copyright (C) 2010, 2013 Nicolas Bonnefon and other contributors
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

#include <QCoreApplication>
#include <QToolButton>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>

#include "persistentinfo.h"
#include "configuration.h"
#include "qfnotifications.h"

#include "quickfindwidget.h"

#include "qt_utils.h"
#include "signal_slot.h"

const int QuickFindWidget::NOTIFICATION_TIMEOUT = 5000;

const QString QFNotification::REACHED_EOF = "Reached end of file, no occurence found.";
const QString QFNotification::REACHED_BOF = "Reached beginning of file, no occurence found.";

QuickFindWidget::QuickFindWidget( QWidget* parent ) : QWidget( parent )
{
    setAutoFillBackground(true);
    // ui_.setupUi( this );
    // setFocusProxy(ui_.findEdit);
    // setProperty("topBorder", true);
    QHBoxLayout *layout = new QHBoxLayout( this );

    layout->setMargin( 0 );
    layout->setSpacing( 6 );

    closeButton_ = setupToolButton(QLatin1String(":/images/close.svg"));
    layout->addWidget( closeButton_ );

    editQuickFind_ = new QComboBox( this );
    editQuickFind_->setEditable( true );
    // FIXME: set MinimumSize might be to constraining
    editQuickFind_->setMinimumSize( QSize( 150, 0 ) );
    editQuickFind_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    editQuickFind_->setMaxCount( 100 );
    layout->addWidget(editQuickFind_, 3);

    previousButton_ = setupToolButton(QLatin1String(":/images/arrowup.svg"));
    layout->addWidget( previousButton_ );

    nextButton_ = setupToolButton(QLatin1String(":/images/arrowdown.svg"));
    layout->addWidget( nextButton_ );

    ignoreCaseCheck_
        = createCheckButton("Ignore case", "Alt+C", ":/images/ignore_case.svg");
    layout->addWidget(ignoreCaseCheck_);

    regexCheck_
        = createCheckButton("Regex search", "Alt+R", ":/images/regex.svg");
    layout->addWidget(regexCheck_);

    incrementalCheck_
        = createCheckButton("Incremental", "Alt+I", ":/images/incremental.svg");
    layout->addWidget(incrementalCheck_);

    notificationText_ = new QLabel( "" );
    // FIXME: set MinimumSize might be too constraining
    int width = QFNotification::maxWidth( notificationText_ );
    notificationText_->setMinimumSize( width, 0 );
    notificationText_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget( notificationText_, 2 );

    setMinimumWidth( minimumSizeHint().width() );

    // Behaviour
    CONNECT(closeButton_, clicked, this, closeHandler);
    connect( editQuickFind_->lineEdit(), &QLineEdit::textEdited, this,
             &QuickFindWidget::textChanged );
    connect(ignoreCaseCheck_, &QPushButton::toggled, [=]() { textChanged(); });
    connect(regexCheck_, &QPushButton::toggled, [=]() { textChanged(); });
    connect(incrementalCheck_, &QPushButton::toggled, [=]() { textChanged(); });
    /*
    connect( editQuickFind_. SIGNAL( textChanged( QString ) ), this,
            SLOT( updateButtons() ) );
    */
    connect( editQuickFind_->lineEdit(), &QLineEdit::returnPressed, this,
             &QuickFindWidget::returnHandler );
    CONNECT(previousButton_, clicked, this, doSearchBackward);
    CONNECT(nextButton_, clicked, this, doSearchForward);

    // Notification timer:
    notificationTimer_ = new QTimer( this );
    notificationTimer_->setSingleShot( true );
    CONNECT(notificationTimer_, timeout, this, notificationTimeout);

    auto config = Persistent<Configuration>( "settings" );
    ignoreCaseCheck_->setChecked(config->quickfindIgnoreCase());
    regexCheck_->setChecked(config->quickfindRegexpType() == ExtendedRegexp);
    incrementalCheck_->setChecked(config->isQuickfindIncremental());
}

void QuickFindWidget::userActivate()
{
    userRequested_ = true;
    QWidget::show();
    editQuickFind_->setFocus( Qt::ShortcutFocusReason );
}

//
// SLOTS
//

void QuickFindWidget::changeDisplayedPattern( const QString& newPattern )
{
    editQuickFind_->lineEdit()->setText( newPattern );
}

void QuickFindWidget::notify( const QFNotification& message )
{
    LOG(logDEBUG) << "QuickFindWidget::notify()";

    notificationText_->setText( message.message() );
    QWidget::show();
    notificationTimer_->start( NOTIFICATION_TIMEOUT );

    // Poor man's asynchronous op.: check for events!
    QCoreApplication::processEvents();
}

void QuickFindWidget::clearNotification()
{
    LOG(logDEBUG) << "QuickFindWidget::clearNotification()";

    notificationText_->setText( "" );
}

// User clicks forward arrow
void QuickFindWidget::doSearchForward()
{
    LOG(logDEBUG) << "QuickFindWidget::doSearchForward()";

    // The user has clicked on a button, so we assume she wants
    // the widget to stay visible.
    userRequested_ = true;

    emit patternConfirmed( editQuickFind_->currentText(), isIgnoreCase() );
    emit searchForward();
}

// User clicks backward arrow
void QuickFindWidget::doSearchBackward()
{
    LOG(logDEBUG) << "QuickFindWidget::doSearchBackward()";

    // The user has clicked on a button, so we assume she wants
    // the widget to stay visible.
    userRequested_ = true;

    emit patternConfirmed( editQuickFind_->currentText(), isIgnoreCase() );
    emit searchBackward();
}

// Close and search when the user presses Return
void QuickFindWidget::returnHandler()
{
    auto text = editQuickFind_->currentText();
    emit patternConfirmed( text, isIgnoreCase() );
    // Close the widget
    userRequested_ = false;
    if ( text == "" )
        this->hide();
    else
        editQuickFind_->insertItem( 0, text );
    emit close();
}

// Close and reset flag when the user clicks 'close'
void QuickFindWidget::closeHandler()
{
    userRequested_ = false;
    this->hide();
    emit close();
    emit cancelSearch();
}

void QuickFindWidget::notificationTimeout()
{
    // We close the widget if the user hasn't explicitely requested it.
    if ( userRequested_ == false )
        this->hide();
}

void QuickFindWidget::textChanged()
{
    auto config = Persistent<Configuration>( "settings" );
    config->setQuickfindIgnoreCase(ignoreCaseCheck_->isChecked());
    config->setQuickfindRegexpType(regexCheck_->isChecked() ? ExtendedRegexp
                                                            : FixedString);
    config->setQuickfindIncremental(incrementalCheck_->isChecked());

    emit patternUpdated( editQuickFind_->currentText(), isIgnoreCase() );
}

//
// Private functions
//
QToolButton* QuickFindWidget::setupToolButton(const QString& icon)
{
    QToolButton *toolButton = new QToolButton(this);

    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon(loadSvgAndAdjustColor(icon)));

    return toolButton;
}

bool QuickFindWidget::isIgnoreCase() const
{
    return ignoreCaseCheck_->isChecked();
}
