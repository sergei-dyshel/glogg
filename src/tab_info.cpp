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

#include "tab_info.h"

#include <QDataStream>

const QString TabInfo::MIME_TYPE = "glogg/tab";

bool TabInfo::tryDecodeMimeData(const QMimeData *data, TabInfo *outTab)
{
    TabInfo tab;
    if (!data->hasFormat(MIME_TYPE))
        return false;
    QDataStream stream(data->data(MIME_TYPE));
    stream >> tab.pid >> tab.tabBar >> tab.tabText >> tab.tabIndex
        >> tab.filename;
    if (stream.status() != QDataStream::Ok)
        return false;
    if (outTab)
        *outTab = tab;
    return true;
}


QMimeData *TabInfo::toMimeData() const
{
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << pid << tabBar << tabText << tabIndex << filename;
    auto data = new QMimeData();
    data->setData(MIME_TYPE, array);
    return data;
}

LogStream &operator<<(LogStream &stream, const TabInfo &tab)
{
    return stream << LOG_MEMBER(&tab, pid) << LOG_MEMBER(&tab, tabBar)
                  << LOG_MEMBER(&tab, tabText) << LOG_MEMBER(&tab, tabIndex)
                  << LOG_MEMBER(&tab, filename);
}
