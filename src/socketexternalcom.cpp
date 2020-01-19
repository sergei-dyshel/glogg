#include "socketexternalcom.h"
#include "externalcom.h"
#include "signal_slot.h"

#include <QLocalServer>
#include <QSharedMemory>

#ifdef Q_OS_UNIX
    #include <signal.h>
    #include <unistd.h>
#endif

#include "log.h"

#ifdef Q_OS_UNIX
QSharedMemory* g_staticSharedMemory = nullptr;

void terminate(int signum)
{
    if (g_staticSharedMemory) {
        delete g_staticSharedMemory;
    }
    ::exit(128 + signum);
}

void setCrashHandler(QSharedMemory* memory)
{
    g_staticSharedMemory = memory;

    // Handle any further termination signals to ensure the
    // QSharedMemory block is deleted even if the process crashes
    signal(SIGSEGV, terminate);
    signal(SIGABRT, terminate);
    signal(SIGFPE,  terminate);
    signal(SIGILL,  terminate);
    signal(SIGINT,  terminate);
    signal(SIGTERM, terminate);
}

#endif

SocketExternalInstance::SocketExternalInstance(const QString &name)
    : ExternalInstance()
{
    socket_.connectToServer(ExternalCommunicator::fullServerName(name));
    if (!socket_.waitForConnected(1000)) {
        LOG( logERROR ) << "Failed to connect to socket";
        throw CantCreateExternalErr();
    }
}

void SocketExternalInstance::loadFile(const QString &file_name)
{
    socket_.write(file_name.toUtf8());
    if (!socket_.waitForBytesWritten(1000)) {
        LOG( logERROR ) << "Failed to send filename";
    }
}

uint32_t SocketExternalInstance::getVersion() const
{
    return 0;
}

SocketExternalCommunicator::SocketExternalCommunicator()
    : ExternalCommunicator()
    , server_(new QLocalServer)
{}

SocketExternalCommunicator::~SocketExternalCommunicator()
{
    if (memory_)
        delete memory_;
    server_->close();
    delete server_;
}

void SocketExternalCommunicator::startListening()
{
    auto fullName = fullServerName(serverName);
    QLocalServer::removeServer(fullName);

    CONNECT(server_, newConnection, this, onConnection);
    if (!server_->listen(fullName)) {
        ERROR << "Can not listen on" << fullName;
        exit(1);
    }
    DEBUG << "Listening on" << fullName;
}

qint32 SocketExternalCommunicator::version() const
{
    return 3;
}

ExternalInstance* SocketExternalCommunicator::otherInstance(const QString &name) const
{
    try {
        return static_cast<ExternalInstance*>( new SocketExternalInstance(name) );
    }
    catch ( CantCreateExternalErr ) {
        LOG(logINFO) << "Cannot find external correspondant, we are the only glogg out there.";
        return nullptr;
    }
}

void SocketExternalCommunicator::onConnection()
{
     QLocalSocket *socket = server_->nextPendingConnection();
     QByteArray data;
     DEBUG << "Incoming connection";
     while(socket->waitForReadyRead(100)) {
         data.append(socket->readAll());
     }

     socket->close();
     DEBUG << "Received data" << data;

     emit loadFile(QString::fromUtf8(data));
}



