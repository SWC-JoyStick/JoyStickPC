#ifndef PCCONVERTER_H
#define PCCONVERTER_H
#include <QObject>
#include <QPointer>
#include <QTcpSocket>
#include <QBuffer>
#include <QRect>

#include "pccontrolevent.h"
#include "pccontroller.h"

/**
 * @brief The PCConverter class
 * PC控制转换类: 继承自控制转换虚基类
 * 将Android发送的Android指令转换成PC/Mac/Linux端的对应指令
 * 将转换后的指令发送给Robot API
 */

class PCConverter: public QObject {
    Q_OBJECT
public:
    PCConverter(QObject *parent = Q_NULLPTR);
    ~PCConverter();
    void setControlSocket(QTcpSocket *controlSocket);       // 获取screen2android的socket

protected:
    quint32 read16(QBuffer &buffer);            // 从buffer读取16bit
    quint32 read32(QBuffer &buffer);            // 从buffer读取32bit
    QRect   readPosition(QBuffer &buffer);      // 从buffer读取mouse指令的位置, 帧大小

private slots:
    void recvControl();
private:
    PCController controller;
    QPointer<QTcpSocket> controlSocket;

    PCControlEvent *message2controlEvent(QByteArray &message);   // 将读取到的字节流控制指令解析为PCControlEvent
};

#endif // PCCONVERTER_H
