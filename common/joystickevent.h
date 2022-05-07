#ifndef JOYSTICKEVENT_H
#define JOYSTICKEVENT_H

#include <QEvent>

class QJoyStickEvent: public QEvent {
    // 用于管理项目中用到的所有自定义事件
public:
    enum EventType {
        DeviceSocket = QEvent::User + 1,        // 设备连接
        SendControl,                            // 发送指令
        RecvControl,                            // 接收指令
    };

    QJoyStickEvent(EventType type): QEvent(QEvent::Type(type)) {}
    ~QJoyStickEvent() {}
};

// DeviceSocketEvent
class DeviceSocketEvent: public QJoyStickEvent {
public:
    DeviceSocketEvent(): QJoyStickEvent(DeviceSocket) {}
    ~DeviceSocketEvent() {}
};

#endif // JOYSTICKEVENT_H
