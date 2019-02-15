/* TODO: add header */

#include "externalcom.h"

#include <memory>

#include <QString>

std::shared_ptr<ExternalCommunicator> externalCommunicator = nullptr;

QString instanceServer = DEFAULT_INSTANCE_NAME;