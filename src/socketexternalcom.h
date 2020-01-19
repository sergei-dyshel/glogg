#ifndef SOCKETEXTERNALCOM_H
#define SOCKETEXTERNALCOM_H

#include "externalcom.h"

#include <QLocalServer>
#include <QSharedMemory>
#include <QLocalSocket>

class SocketExternalInstance : public ExternalInstance
{
public:
    SocketExternalInstance(const QString &name);

    void loadFile( const QString& file_name ) override;
    uint32_t getVersion() const override;
private:
    QLocalSocket socket_;
};

class SocketExternalCommunicator : public ExternalCommunicator
{
    Q_OBJECT
public:
    SocketExternalCommunicator();
    ~SocketExternalCommunicator();

    ExternalInstance* otherInstance(const QString &name) const override;
    void startListening() override;

public slots:
    qint32 version() const override;

private slots:
    void onConnection();

private:
     QSharedMemory* memory_ = nullptr;
     QLocalServer* server_;
};

#endif // SOCKETEXTERNALCOM_H
