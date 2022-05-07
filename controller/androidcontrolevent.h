#ifndef ANDROIDCONTROLEVENT_H
#define ANDROIDCONTROLEVENT_H

#include <QRect>
#include <QBuffer>
#include "input.h"
#include "keycodes.h"
#include "joystickevent.h"

/**
 * @brief The AndroidControlEvent class
 * 提供接口, 在PC端生成Android可以理解的指令, 以供发送到Android端来控制对应设备
 */

class AndroidControlEvent: public QJoyStickEvent {
public:
    enum ControlEventType {                             // 指令类型
        CET_KEYCODE = 0,
        CET_TEXT,
        CET_MOUSE,
        CET_SCROLL,
        CET_COMMAND,
        CET_TOUCH,
    };

    AndroidControlEvent(ControlEventType type);
    ~AndroidControlEvent();

    void setKeycodeEventData(AndroidKeyeventAction action, AndroidKeycode keycode, AndroidMetastate metastate);     // 设置keycode指令的参数
    void setTextEventData(QString text);                                                                            // 设置text指令的参数
    void setMouseEventData(AndroidMotioneventAction action, AndroidMotioneventButtons buttons, QRect position);     // 设置mouse指令的参数
    void setScrollEventData(QRect position, qint32 hScroll, qint32 vScroll);                                        // 设置scroll指令的参数
    void setCommandEventData(qint32 action);                                                                        // 设置command指令的参数
    void setTouchEventData(quint32 id, AndroidMotioneventAction action, QRect position);                            // 设置touch指令的参数
    QByteArray serializedData();                        // 将指令数据序列化为QByteArray以便发送

protected:
    void write16(QBuffer &buffer, quint32 value);       // buffer写入16bit
    void write32(QBuffer &buffer, quint32 value);       // buffer写入32bit
    void writePosition(QBuffer &buffer, QRect &value);  // buffer写入mouse指令的位置、帧大小
private:
    struct ControlEventData {
        ControlEventType type;
        union {
            struct {
                AndroidKeyeventAction action;
                AndroidKeycode keycode;
                AndroidMetastate metastate;
            } keycodeEvent;
            struct {
                QString text;
            } textEvent;
            struct {
                AndroidMotioneventAction action;        // 鼠标动作类型
                AndroidMotioneventButtons buttons;      // 对应的按键
                QRect position;                         // 点击位置
            } mouseEvent;
            struct {
                QRect positon;
                qint32 hScroll;
                qint32 vScroll;
            } scrollEvent;
            struct {
                qint32 action;
            } commandEvent;
            struct {
                quint32 id;
                AndroidMotioneventAction action;
                QRect position;
            } touchEvent;
        };
        ControlEventData() {}
        ~ControlEventData() {}
    };

    ControlEventData data;
};

#endif // ANDROIDCONTROLEVENT_H
