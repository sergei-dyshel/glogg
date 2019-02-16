/* TODO: add header */

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