#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class TcpServer: public QTcpServer {
    Q_OBJECT
public:
    TcpServer(QObject *parent = Q_NULLPTR);
    ~TcpServer();

protected:
    virtual void incomingConnection(qintptr handle);
};

#endif // TCPSERVER_H
