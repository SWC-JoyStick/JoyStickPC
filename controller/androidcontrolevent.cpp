#include <QBuffer>

#include "androidcontrolevent.h"

#define TEXT_MAX_CHARACTER_LENGTH 300

AndroidControlEvent::AndroidControlEvent(ControlEventType type): QJoyStickEvent(QJoyStickEvent::SendControl) {
    data.type = type;
}

AndroidControlEvent::~AndroidControlEvent() {

}

void AndroidControlEvent::setKeycodeEventData(AndroidKeyeventAction action, AndroidKeycode keycode, AndroidMetastate metastate) {
    data.keycodeEvent.action = action;
    data.keycodeEvent.keycode = keycode;
    data.keycodeEvent.metastate = metastate;
}

void AndroidControlEvent::setTextEventData(QString text) {
    data.textEvent.text = text;
}

void AndroidControlEvent::setMouseEventData(AndroidMotioneventAction action, AndroidMotioneventButtons buttons, QRect position) {
    data.mouseEvent.action = action;
    data.mouseEvent.buttons = buttons;
    data.mouseEvent.position = position;
}

void AndroidControlEvent::setScrollEventData(QRect position, qint32 hScroll, qint32 vScroll) {
    data.scrollEvent.positon = position;
    data.scrollEvent.hScroll = hScroll;
    data.scrollEvent.vScroll = vScroll;
}

void AndroidControlEvent::setCommandEventData(qint32 action) {
    data.commandEvent.action = action;
}

void AndroidControlEvent::setTouchEventData(quint32 id, AndroidMotioneventAction action, QRect position) {
    data.touchEvent.id = id;
    data.touchEvent.action = action;
    data.touchEvent.position = position;
}

QByteArray AndroidControlEvent::serializedData() {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);                                 // 利用QBuffer构造控制指令
    buffer.open(QBuffer::WriteOnly);
    // 将操作存入buffer
    buffer.putChar(data.type);                                  // 设备类型: mouse/touch...
    switch (data.type) {                                        // 操作类型: up/down...
        case CET_KEYCODE:
            buffer.putChar(data.keycodeEvent.action);
            write32(buffer, data.keycodeEvent.keycode);
            write32(buffer, data.keycodeEvent.metastate);
            break;
        case CET_TEXT: {
            if (TEXT_MAX_CHARACTER_LENGTH < data.textEvent.text.length())
                data.textEvent.text = data.textEvent.text.left(TEXT_MAX_CHARACTER_LENGTH);
            QByteArray tmp = data.textEvent.text.toUtf8();
            write16(buffer, tmp.length());
            buffer.write(tmp.data(), tmp.length());
            break;
        }
        case CET_MOUSE:
            buffer.putChar(data.mouseEvent.action);
            write32(buffer, data.mouseEvent.buttons);
            writePosition(buffer, data.mouseEvent.position);
            break;
        case CET_SCROLL:
            writePosition(buffer, data.scrollEvent.positon);
            write32(buffer, data.scrollEvent.hScroll);
            write32(buffer, data.scrollEvent.vScroll);
            break;
        case CET_COMMAND:
            buffer.putChar(data.commandEvent.action);
            break;
        case CET_TOUCH:
            buffer.putChar(data.touchEvent.id);
            buffer.putChar(data.touchEvent.action);
            writePosition(buffer, data.touchEvent.position);
            break;
        default:
            break;
    }
    buffer.close();
    return byteArray;
}

void AndroidControlEvent::write16(QBuffer &buffer, quint32 value) {
    // value: 大小 2Byte
    buffer.putChar(value >> 8);
    buffer.putChar(value);
}

void AndroidControlEvent::write32(QBuffer &buffer, quint32 value) {
    // value: 按下的键 4Byte
    buffer.putChar(value >> 24);
    buffer.putChar(value >> 16);
    buffer.putChar(value >> 8);
    buffer.putChar(value);
}

void AndroidControlEvent::writePosition(QBuffer &buffer, QRect &value) {
    write16(buffer, value.left());
    write16(buffer, value.top());
    write16(buffer, value.width());
    write16(buffer, value.height());
}
