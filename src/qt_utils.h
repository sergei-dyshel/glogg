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

#include "color_scheme.h"

#include <QWidget>

class QAbstractButton;
class QToolButton;
class QPixmap;

inline void disableTabFocus(QWidget* widget)
{
    widget->setFocusPolicy(
        static_cast<Qt::FocusPolicy>(widget->focusPolicy() & ~Qt::TabFocus));
}

template <typename T> void disableTabFocusOnChildren(QWidget* widget)
{
    for (auto child : widget->findChildren<T*>())
        disableTabFocus(child);
}

void addColorIconToAction(QAction* action, const TextColor& color);

void addColorIconToAction(QAction* action, const QColor &color);

void setButtonToolTipWithShortcut(QAbstractButton& button,
                                  const QString& toolTip);

QToolButton* createCheckButton(const QString& tooltip, const QString& shortcut,
                               const QString& iconPath);

QPixmap loadPngAndAdjustColor(const QString& filename);
QPixmap loadSvgAndAdjustColor(const QString &filename);
