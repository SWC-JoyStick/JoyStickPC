#include "sender.h"

Sender:: Sender(PacketShared *packet) {
    server = new QTcpServer(this);
    this->packet = packet;
    this->isRunning = true;

    connect(server, &QTcpServer::newConnection, this, [this]() {
        this->socket = server->nextPendingConnection();
        qDebug() << "Connection Success";
        emit onConnected();
    });
}

void Sender::startListening() {
    if (!server->listen(QHostAddress::Any, SENDPORT)) {
        // 监听IP+Port，等待Android连接
        qDebug() << server->errorString();
    }
    qDebug() << "Listening...";
}

void Sender::stop() {
    if (server->isListening())
        server->disconnect();
    server->close();
}

QPointer<QTcpSocket> Sender::getDeviceSocket() const {
    return this->socket;
}

void Sender::stopSending() {
    this->isRunning = false;
}

void Sender::run() {
    while (isRunning) {
        // packet的消费者
        packet->full.acquire();
        this->send();
        packet->empty.release();
    }
}

void Sender::send() {
    QByteArray buffer = QByteArray::fromRawData((const char *)packet->getPacket()->data, packet->getPacket()->size);
    qDebug() << "packet size = " << buffer.size();
    int ret = this->socket->write(buffer);
    if (ret == -1) {
        qDebug() << this->socket->errorString();
        isRunning = false;
        qDebug() << "Connection closed";
        emit onDisconnected();
        return;
    }
    this->socket->waitForBytesWritten();
}

