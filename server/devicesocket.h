#ifndef DEVICESOCKET_H
#define DEVICESOCKET_H

#include <QTcpSocket>
#include <QWaitCondition>
#include <QMutex>

/* 使用生产者/消费者模型接收视频流数据
 * DeviceSocket是PC端的消费者
 */

class DeviceSocket: public QTcpSocket {
    Q_OBJECT
public:
    DeviceSocket(QObject *parent = Q_NULLPTR);
    ~DeviceSocket();

    qint32 subThreadRecvData(quint8 *buf, qint32 bufSize);          // 消费者函数, 将内部buffer指向外部buf, 等待生产者将数据读入到外部的buf, 然后返回

protected:
    bool event(QEvent *event);

protected slots:
    void onReadyRead();                                             // 生产者函数, 等待readyRead信号, 将视频流读入buffer
    void quitNotify();                                              // 主动退出消费者线程, 即解码线程

private:
    // 多线程编程, 添加条件变量和互斥锁
    QMutex mutex;
    QWaitCondition recvDataCondition;
    // 若干标记
    bool isReceived = false;                    // 是否已经收到数据
    bool isQuit = false;                        // 是否退出

    quint8 *buffer = Q_NULLPTR;                 // 视频流的缓冲区, 需要互斥访问, 因此加锁, 采用生产者/消费者模型访问buffer
    quint32 bufferSize = 0;                     // 缓冲区大小
    quint32 dataSize = 0;                       // 实际收到数据个数
};

#endif // DEVICESOCKET_H
