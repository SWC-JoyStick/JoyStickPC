#ifndef BONJOUR_H
#define BONJOUR_H
#include <QObject>
#include <QtNetwork>
#define RECVPORT 45454

class Bonjour: public QObject {
    Q_OBJECT
public:
    explicit Bonjour(QObject *parent = nullptr);
    ~Bonjour();
    QPointer<QUdpSocket> getSender() const;
    QPointer<QUdpSocket> getReceiver() const;
    QHostAddress getLocalHostIP() const;
    QString getHostname() const;
    void start();
    void stop();

signals:
    void sendSelfInfo(QString phoneIP);
    void updatePhoneInfo(QString phoneName, QString phoneIP);

private:
    QPointer<QUdpSocket> sender;
    QPointer<QUdpSocket> receiver;
    QHostAddress localhostip;
    QString hostname;

private slots:
    void processPendingDatagram();
    void sendLocalHostInfo(QString phoneIP);
};

#endif // BONJOUR_H
