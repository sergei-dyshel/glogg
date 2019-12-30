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

#include "externalcom.h"

#include <memory>

#include <QString>

std::shared_ptr<ExternalCommunicator> externalCommunicator = nullptr;

const QString ExternalCommunicator::DEFAULT_SERVER_NAME = "default";
const QString ExternalCommunicator::SERVER_NAME_PREFIX = "org.bonnefon.glogg";
QString ExternalCommunicator::serverName = ExternalCommunicator::DEFAULT_SERVER_NAME;

QString ExternalCommunicator::fullServerName(const QString& name)
{
    auto result = SERVER_NAME_PREFIX;
    if (!name.isEmpty())
        result += "." + name;
    return result;
}
