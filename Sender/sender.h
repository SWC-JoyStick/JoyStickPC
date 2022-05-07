#ifndef SENDER_H
#define SENDER_H

#include <QtNetwork>
#include <QThread>
#include "packetshared.h"

#define SENDPORT 6666

typedef AVPacket AVPacket;

class Sender: public QThread {
    Q_OBJECT
public:
    Sender(PacketShared *packet);
    void startListening();
    void stop();
    QPointer<QTcpSocket> getDeviceSocket() const;

public slots:
    void stopSending();

signals:
    void onConnected();
    void onDisconnected();

protected:
    void run();

private:
    PacketShared *packet;
    QPointer<QTcpServer> server;
    QPointer<QTcpSocket> socket;
    bool isRunning;

    void send();
};

#endif // SENDER_H
