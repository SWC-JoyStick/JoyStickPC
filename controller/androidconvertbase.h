#ifndef ANDROIDCONVERTBASE_H
#define ANDROIDCONVERTBASE_H
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include "androidcontroller.h"

/* 控制转换虚基类, 提供必要的连接socket, 事件发送接口
 * 用于将PC端的动作/操作转换为Android的指令, 从而实现PC端控制Android设备
 */

class AndroidConvertBase {
public:
    AndroidConvertBase();
    virtual ~AndroidConvertBase();

    virtual void mouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize) = 0;
    virtual void wheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize) = 0;
    virtual void keyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize) = 0;

    void setControlSocket(DeviceSocket *controlSocket);

protected:
    void sendControlEvent(AndroidControlEvent *event);

private:
    AndroidController controller;
};

inline AndroidConvertBase::AndroidConvertBase() {

}

inline AndroidConvertBase::~AndroidConvertBase() {

}

inline void AndroidConvertBase::setControlSocket(DeviceSocket *controlSocket) {
    controller.setControlSocket(controlSocket);
}

inline void AndroidConvertBase::sendControlEvent(AndroidControlEvent *event) {
    if (event)
        controller.postControlEvent(event);
}


#endif // ANDROIDCONVERTBASE_H


