#include "pcconverter.h"

PCConverter::PCConverter(QObject *parent): QObject(parent), controller(this) {
}

PCConverter::~PCConverter() {
}

void PCConverter::setControlSocket(QTcpSocket *controlSocket) {
    this->controlSocket = controlSocket;
    connect(this->controlSocket, &QTcpSocket::readyRead, this, &PCConverter::recvControl);
}

quint32 PCConverter::read16(QBuffer &buffer) {
    // 读取2Byte
    quint32 value = 0x0;
    char byte = 0x0;
    for (int i = 1; i >= 0; i--) {
        buffer.getChar(&byte);
        value |= (byte << (8 * i));
    }
    return value;
}

quint32 PCConverter::read32(QBuffer &buffer) {
    quint32 value = 0x0;
    char byte = 0x0;
    for (int i = 3; i >= 0; i--) {
        buffer.getChar(&byte);
        value |= (byte << (8 * i));
    }
    return value;
}

QRect PCConverter::readPosition(QBuffer &buffer) {
    QRect pos;
    pos.setLeft(read16(buffer));
    pos.setTop(read16(buffer));
    pos.setWidth(read16(buffer));
    pos.setHeight(read16(buffer));
    return pos;
}

void PCConverter::recvControl() {
    // 读取控制指令
    QByteArray controlMessage = controlSocket->readAll();
    PCControlEvent *controlEvent = message2controlEvent(controlMessage);
    controller.postControlEvent(controlEvent);
}

PCControlEvent *PCConverter::message2controlEvent(QByteArray &message) {
    QBuffer buffer(&message);
    char c_char;
    // 读取type字段, 建立PCControlEvent
    buffer.getChar(&c_char);
    ControlEventType type = ControlEventType(c_char);
    PCControlEvent *controlEvent = new PCControlEvent(type);
    switch (type) {
        case CET_KEYCODE:
            buffer.getChar(&c_char);
            controlEvent->data.keycodeEvent.action = (AndroidKeyeventAction)c_char;
            controlEvent->data.keycodeEvent.keycode = (AndroidKeycode)read32(buffer);
            controlEvent->data.keycodeEvent.metastate = (AndroidMetastate)read32(buffer);
            break;
        case CET_TEXT:
            break;
        case CET_MOUSE:
            buffer.getChar(&c_char);
            controlEvent->data.mouseEvent.action = (AndroidMotioneventAction)c_char;
            controlEvent->data.mouseEvent.buttons = (AndroidMotioneventButtons)read32(buffer);
            controlEvent->data.mouseEvent.position = readPosition(buffer);
            break;
        case CET_SCROLL:
            break;
        case CET_COMMAND:
            break;
        case CET_TOUCH:
            buffer.getChar(&c_char);
            controlEvent->data.touchEvent.id = (quint32)c_char;
            buffer.getChar(&c_char);
            controlEvent->data.touchEvent.action = (AndroidMotioneventAction)c_char;
            controlEvent->data.touchEvent.position = readPosition(buffer);
            break;
        default:
            break;
    }
    return controlEvent;
}
