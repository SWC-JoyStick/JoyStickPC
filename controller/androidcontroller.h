#ifndef ANDROIDCONTROLLER_H
#define ANDROIDCONTROLLER_H
#include <QObject>
#include <QPointer>
#include "devicesocket.h"
#include "androidcontrolevent.h"

/* 管理控制指令发送/接收的类
 * PC ===> Android: 用于将控制指令从PC端发送到Android端
 */

class AndroidController: public QObject {
    Q_OBJECT
public:
    AndroidController(QObject *parent = Q_NULLPTR);
    ~AndroidController();

    void setControlSocket(DeviceSocket *controlSocket);         // 设置用于发送指令的socket
    void postControlEvent(AndroidControlEvent *controlEvent);   // 将指令发送事件post出去
    void test(QRect rc);

protected:
    bool event(QEvent *event);

private:
    bool sendControl(const QByteArray &buffer);                 // 发送指令到socket

    QPointer<DeviceSocket> controlSocket = Q_NULLPTR;           // 用于发送/接收指令的socket
};

#endif // ANDROIDCONTROLLER_H
