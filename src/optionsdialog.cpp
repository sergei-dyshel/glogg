/*
 * Copyright (C) 2009, 2010, 2011, 2013 Nicolas Bonnefon and other contributors
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

#include <QtGui>

#include "optionsdialog.h"

#include "log.h"
#include "persistentinfo.h"
#include "configuration.h"
#include "signal_slot.h"

static const uint32_t POLL_INTERVAL_MIN = 10;
static const uint32_t POLL_INTERVAL_MAX = 3600000;

// Constructor
OptionsDialog::OptionsDialog( QWidget* parent ) : QDialog(parent)
{
    setupUi( this );

    setupTabs();

    // Validators
    QValidator* polling_interval_validator_ = new QIntValidator(
           POLL_INTERVAL_MIN, POLL_INTERVAL_MAX, this );
    pollIntervalLineEdit->setValidator( polling_interval_validator_ );

    CONNECT(buttonBox, clicked, this, onButtonBoxClicked);
    CONNECT(pollingCheckBox, toggled, this, onPollingChanged);

    updateDialogFromConfig();

    setupPolling();
}

//
// Private functions
//

// Setups the tabs depending on the configuration
void OptionsDialog::setupTabs()
{
#ifndef GLOGG_SUPPORTS_POLLING
    tabWidget->removeTab( 1 );
#endif
}

void OptionsDialog::setupPolling()
{
    pollIntervalLineEdit->setEnabled( pollingCheckBox->isChecked() );
}

// Updates the dialog box using values in global Config()
void OptionsDialog::updateDialogFromConfig()
{
    std::shared_ptr<Configuration> config =
        Persistent<Configuration>( "settings" );

    // Polling
    pollingCheckBox->setChecked( config->pollingEnabled() );
    pollIntervalLineEdit->setText( QString::number( config->pollIntervalMs() ) );

    // Last session
    loadLastSessionCheckBox->setChecked( config->loadLastSession() );
}

//
// Slots
//

void OptionsDialog::updateConfigFromDialog()
{
    std::shared_ptr<Configuration> config =
        Persistent<Configuration>( "settings" );

    config->setPollingEnabled( pollingCheckBox->isChecked() );
    uint32_t poll_interval = pollIntervalLineEdit->text().toUInt();
    if ( poll_interval < POLL_INTERVAL_MIN )
        poll_interval = POLL_INTERVAL_MIN;
    else if (poll_interval > POLL_INTERVAL_MAX )
        poll_interval = POLL_INTERVAL_MAX;

    config->setPollIntervalMs( poll_interval );

    config->setLoadLastSession( loadLastSessionCheckBox->isChecked() );
    emit optionsChanged();
}

void OptionsDialog::onButtonBoxClicked( QAbstractButton* button )
{
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole( button );
    if (   ( role == QDialogButtonBox::AcceptRole )
        || ( role == QDialogButtonBox::ApplyRole ) ) {
        updateConfigFromDialog();
    }

    if ( role == QDialogButtonBox::AcceptRole )
        accept();
    else if ( role == QDialogButtonBox::RejectRole )
        reject();
}

void OptionsDialog::onPollingChanged()
{
    setupPolling();
}
