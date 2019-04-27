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

#include "qt_utils.h"

#include <QAction>
#include <QPainter>
#include <QAbstractButton>
#include <QPushButton>

void addColorIconToAction(QAction* action, const TextColor& color)
{
    QPixmap pixmap(100, 100);
    pixmap.fill(color.background.isValid() ? color.background
                                            : color.foreground);
    if (color.foreground.isValid()) {
        QPainter painter(&pixmap);
        painter.setPen(color.foreground);
        painter.setBrush(color.foreground);
        painter.drawEllipse(25, 25, 50, 50);
    }
    action->setIcon(pixmap);
    action->setIconVisibleInMenu(true);
}

void addColorIconToAction(QAction* action, const QColor& color)
{
    addColorIconToAction(action, TextColor(color));
}

void setButtonToolTipWithShortcut(QAbstractButton& button,
                                  const QString& toolTip)
{
    button.setToolTip(
        QString( "%1 (%2)" )
            .arg( toolTip )
            .arg( button.shortcut().toString( QKeySequence::NativeText ) ) );
}

QPushButton* createCheckButton(const QString& tooltip, const QString& shortcut,
                               const QString& iconPath)
{
    QPushButton* button = new QPushButton();
    button->setCheckable(true);
    button->setFlat(true);
    button->setShortcut(QKeySequence(shortcut));
    setButtonToolTipWithShortcut(*button, tooltip);
    button->setIcon(QIcon(iconPath));
    return button;
}