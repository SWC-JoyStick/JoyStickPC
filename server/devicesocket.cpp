#include <QCoreApplication>
#include <QThread>
#include <QMutexLocker>
#include "devicesocket.h"
#include "joystickevent.h"

DeviceSocket::DeviceSocket(QObject *parent): QTcpSocket(parent) {
    connect(this, &DeviceSocket::readyRead, this, &DeviceSocket::onReadyRead);
    connect(this, &DeviceSocket::disconnected, this, &DeviceSocket::quitNotify);
    connect(this, &DeviceSocket::aboutToClose, this, &DeviceSocket::quitNotify);
}

DeviceSocket::~DeviceSocket() {

}

qint32 DeviceSocket::subThreadRecvData(quint8 *buf, qint32 bufSize) {
    // 确保当前线程不是主线程, 仅使用子线程接收视频流
    Q_ASSERT(QCoreApplication::instance()->thread() != QThread::currentThread());
    if (isQuit)
        return 0;
    /* QMutexLocker 建立时 mutex.lock()
     * QMutexLocker 销毁时 mutex.unlock()
     */
    QMutexLocker locker(&mutex);

    buffer = buf;
    bufferSize = bufSize;
    dataSize = 0;

    // 发送DeviceSocketEvent
    DeviceSocketEvent *getDataEvent = new DeviceSocketEvent();
    QCoreApplication::postEvent(this, getDataEvent);

    // 消费者阻塞等待生产者
    while (!isReceived)
        recvDataCondition.wait(&mutex);

    isReceived = false;
    return dataSize;
}

bool DeviceSocket::event(QEvent *event) {
    if (event->type() == DeviceSocketEvent::DeviceSocket) {
        onReadyRead();
        return true;
    }
    return QTcpSocket::event(event);
}

void DeviceSocket::onReadyRead() {
    QMutexLocker locker(&mutex);
    if (buffer && bytesAvailable() > 0) {
        // 接收数据
        qint64 readSize = qMin(bytesAvailable(), (qint64)bufferSize);
        dataSize = read((char *)buffer, readSize);

        buffer = Q_NULLPTR;
        bufferSize = 0;
        isReceived = true;
        recvDataCondition.wakeOne();
    }
}

void DeviceSocket::quitNotify() {
    isQuit = true;
    QMutexLocker locker(&mutex);
    if (buffer) {       // buffer不为空, 说明消费者阻塞
        buffer = Q_NULLPTR;
        bufferSize = 0;
        isReceived = true;
        dataSize = 0;
        recvDataCondition.wakeOne();
    }
}
