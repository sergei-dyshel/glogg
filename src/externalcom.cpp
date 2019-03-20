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

static const QStringList predefinedNames = {"default", "a", "b", "c"};

const QString ExternalCommunicator::DEFAULT_SERVER_NAME = "default";

void ExternalCommunicator::startServer(const QString &name)
{
    auto all_servers = allServerNames();
    if (!name.isEmpty()) {
        assert(!all_servers.contains(name));
        serverName_ = name;
    }
    else {
        for (auto predefined : predefinedNames)
            if (!all_servers.contains(predefined)) {
                serverName_ = predefined;
                break;
            }
        assert(!serverName_.isEmpty());
    }
    startListening(serverName_);
}

QStringList ExternalCommunicator::otherServerNames() const
{
    QStringList result;
    for (auto name : allServerNames())
        if (name != serverName_)
            result.append(name);
    return result;
}