#include "socketexternalcom.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QSharedMemory>

#ifdef Q_OS_UNIX
    #include <signal.h>
    #include <unistd.h>
#endif

#include "log.h"

static const QString GLOG_SERVICE_NAME = "org.bonnefon.glogg";

#ifdef Q_OS_UNIX
QSharedMemory* g_staticSharedMemory = nullptr;

static QString fullServiceName(const QString& name)
{
    auto result = GLOG_SERVICE_NAME;
    if (!name.isEmpty())
        result += "." + name;
    return result;
}

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
    : ExternalInstance(), memory_(new QSharedMemory(fullServiceName(name) ))
{
    if ( !memory_->attach( QSharedMemory::ReadOnly ) ) {
        LOG( logERROR ) << "attach failed!";
        throw CantCreateExternalErr();
    }

#ifdef Q_OS_UNIX
        // Handle any further termination signals to ensure the
        // QSharedMemory block is deleted even if the process crashes
        setCrashHandler(memory_);
#endif
}

void SocketExternalInstance::loadFile(const QString &file_name) const
{
    QLocalSocket socket;
    socket.connectToServer(memory_->key());
    if (!socket.waitForConnected(1000)) {
        LOG( logERROR ) << "Failed to connect to socket";
        return;
    }

    socket.write(file_name.toUtf8());
    if (!socket.waitForBytesWritten(1000)) {
        LOG( logERROR ) << "Failed to send filename";
    }

    socket.close();
}

uint32_t SocketExternalInstance::getVersion() const
{
    return *reinterpret_cast<uint32_t*>(memory_->data());
}

SocketExternalCommunicator::SocketExternalCommunicator()
    : ExternalCommunicator()
    , server_(new QLocalServer)
{}

SocketExternalCommunicator::~SocketExternalCommunicator()
{
    delete memory_;
    server_->close();
    delete server_;
}

QStringList SocketExternalCommunicator::allServerNames() const {
    return QStringList();
}

void SocketExternalCommunicator::startListening(const QString &name)
{
    auto fullName = fullServiceName(name);
    memory_ = new QSharedMemory(fullName);
    if ( memory_->create(sizeof(qint32))) {
#ifdef Q_OS_UNIX
        // Handle any further termination signals to ensure the
        // QSharedMemory block is deleted even if the process crashes
        setCrashHandler(memory_);
#endif

        *reinterpret_cast<qint32*>(memory_->data()) = version();
        QLocalServer::removeServer(fullName);

        connect(server_, SIGNAL(newConnection()), SLOT(onConnection()));
        server_->listen(fullName);
    }
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
     while(socket->waitForReadyRead(100)) {
         data.append(socket->readAll());
     }

     socket->close();

     emit loadFile(QString::fromUtf8(data));
}



