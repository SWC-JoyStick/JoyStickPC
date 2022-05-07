#ifndef ANDROIDCONVERTNORMAL_H
#define ANDROIDCONVERTNORMAL_H

#include "androidconvertbase.h"

/**
 * Android控制转换类: 继承自控制转换虚基类
 * 将PC端的mouseEvent, wheelEvent, keyEvent转换为Android端的对应指令
 */

class AndroidConvertNormal: public AndroidConvertBase {
public:
    AndroidConvertNormal();
    virtual ~AndroidConvertNormal();

    virtual void mouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize);        // 将PC的mouseEvent转换成Android的event
    virtual void wheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize);        // 将PC的wheelEvent转换成Android的event
    virtual void keyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize);            // 将PC的keyEvent转换成Android的event

private:
    AndroidMotioneventButtons qtButtons2android(Qt::MouseButtons buttonState);
    AndroidKeycode qtKeycode2android(qint32 key, Qt::KeyboardModifiers modifiers);
    AndroidMetastate modifier2metastate(Qt::KeyboardModifiers modifiers);
};

#endif // ANDROIDCONVERTNORMAL_H
