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
#include <QToolButton>
#include <QPixmap>
#include <QGuiApplication>
#include <QBitmap>
#include <QGraphicsSvgItem>
#include <QGraphicsColorizeEffect>
#include <QPainter>
#include <QBitmap>
#include <QStyleOptionGraphicsItem>

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

QToolButton* createCheckButton(const QString& tooltip, const QString& shortcut,
                               const QString& iconPath)
{
    QToolButton* button = new QToolButton();
    button->setCheckable(true);
    button->setShortcut(QKeySequence(shortcut));
    setButtonToolTipWithShortcut(*button, tooltip);
    button->setIcon(QIcon(loadSvgAndAdjustColor(iconPath)));
    return button;
}

QPixmap loadPngAndAdjustColor(const QString& filename)
{
    QPixmap pixmap(filename);
    auto mask = pixmap.createMaskFromColor(Qt::black, Qt::MaskOutColor);
    pixmap.fill(QGuiApplication::palette().color(QPalette::Text));
    pixmap.setMask(mask);

    return pixmap;
}

QPixmap loadSvgAndAdjustColor(const QString &filename)
{
    QGraphicsSvgItem *item = new QGraphicsSvgItem(filename);

    QGraphicsColorizeEffect effect;

    effect.setColor(QGuiApplication::palette().color(QPalette::Text));
    effect.setStrength(1);

    item->setGraphicsEffect(&effect);

    QPixmap pixmap(item->boundingRect().size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    item->paint(&painter, &option);
    return pixmap;
}
