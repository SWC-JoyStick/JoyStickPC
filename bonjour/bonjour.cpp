#include "bonjour.h"

Bonjour::Bonjour(QObject *parent): QObject{parent} {
    for (const QHostAddress &address : QNetworkInterface::allAddresses()) {
        if (address.protocol() == QHostAddress::IPv4Protocol && address != QHostAddress::LocalHost) {
            localhostip = address;
            break;
        }
    }
    hostname = QHostInfo::localHostName();
}

Bonjour::~Bonjour() {
    if (!sender.isNull())
        sender->close();
    if (!sender.isNull())
        receiver->close();
}

QPointer<QUdpSocket> Bonjour::getSender() const {
    return receiver;
}

QPointer<QUdpSocket> Bonjour::getReceiver() const {
    return receiver;
}

QHostAddress Bonjour::getLocalHostIP() const {
    return localhostip;
}

QString Bonjour::getHostname() const{
    return hostname;
}

void Bonjour::start() {
    // 接收方
    receiver = new QUdpSocket(this);
    receiver->bind(RECVPORT, QUdpSocket::ShareAddress);
    qDebug() << "Udp Listening...";
    // 发送方
    sender = new QUdpSocket(this);
    connect(receiver, &QUdpSocket::readyRead, this, &Bonjour::processPendingDatagram);
    connect(this, &Bonjour::sendSelfInfo, this, &Bonjour::sendLocalHostInfo);
}

void Bonjour::stop() {
    if (!sender.isNull())
        sender->close();
    if (!receiver.isNull())
        receiver->close();
}

void Bonjour::processPendingDatagram() {
    // 拥有等待的数据包
    while(receiver->hasPendingDatagrams()){
        // 用于存放接受的数据包
        QByteArray datagram;
        // 让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        datagram.resize(receiver->pendingDatagramSize());
        // 接收数据报，将其存放到datagram中
        receiver->readDatagram(datagram.data(), datagram.size());
        // 将数据报内容显示出来
        qDebug() << datagram;

        // ui->label->setText(datagram);
        // 收到报文 "DISCOVER_REQUEST"+'\n' + deviceName +'\n' +IP
        // 按照'\n' 分割 取deviceName + IP 作为一个QString 处理完后发射信号
        QStringList phoneInfo = QString(datagram).split('\n');
        if (phoneInfo.size() < 3)
            qDebug() << "Invalid Datagram\n";

        if (phoneInfo[0] == "DISCOVER_RESPONSE")
            return;
        // 现在name
        for (QString &info : phoneInfo)
            qDebug() << info;
        // 发射 Name + IP
        // 根据手机端要求
        // 收到DISCOVER_REQUEST 回复报文
        // 收到DISCOVER_RESPONSE 不回复
        if (phoneInfo[0] == "DISCOVER_REQUEST")
            emit sendSelfInfo(phoneInfo[2]);

        emit updatePhoneInfo(phoneInfo[1], phoneInfo[2]);
    }
}

void Bonjour::sendLocalHostInfo(QString phoneIP) {
    QString message = "DISCOVER_RESPONSE\n" + this->hostname + "\n" + this->localhostip.toString();
    QByteArray datagram = message.toLocal8Bit();
    sender->writeDatagram(datagram, QHostAddress(phoneIP), RECVPORT);
}
