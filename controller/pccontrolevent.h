#ifndef PCCONTROLEVENT_H
#define PCCONTROLEVENT_H

#include <QRect>
#include <QBuffer>
#include "input.h"
#include "keycodes.h"
#include "joystickevent.h"

/**
 * @brief The PCControlEvent class
 * 根据接收到的Android指令转换生成PC/Mac/Linux控制指令
 */

enum ControlEventType {                     // 指令类型
    CET_KEYCODE = 0,
    CET_TEXT,
    CET_MOUSE,
    CET_SCROLL,
    CET_COMMAND,
    CET_TOUCH,
};

class PCControlEvent: public QJoyStickEvent {
public:
    PCControlEvent(ControlEventType type);
    ~PCControlEvent();

public:
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

#endif // PCCONTROLEVENT_H
