#include <QCoreApplication>

#include "androidcontroller.h"
#include "androidcontrolevent.h"

AndroidController::AndroidController(QObject *parent): QObject(parent) {

}

AndroidController::~AndroidController() {

}

void AndroidController::setControlSocket(DeviceSocket *controlSocket) {
    this->controlSocket = controlSocket;
}

void AndroidController::postControlEvent(AndroidControlEvent *controlEvent) {
    if (controlEvent)
        QCoreApplication::postEvent(this, controlEvent);

}

void AndroidController::test(QRect rc) {
    AndroidControlEvent *mouseEvent = new AndroidControlEvent(AndroidControlEvent::CET_MOUSE);
    mouseEvent->setMouseEventData(AMOTION_EVENT_ACTION_DOWN, AMOTION_EVENT_BUTTON_PRIMARY, rc);
    postControlEvent(mouseEvent);
}

bool AndroidController::event(QEvent *event) {
    if (event && event->type() == AndroidControlEvent::SendControl) {
        AndroidControlEvent *sendEvent = dynamic_cast<AndroidControlEvent *>(event);
        if (sendEvent)
            sendControl(sendEvent->serializedData());
        return true;
    }
    return QObject::event(event);
}

bool AndroidController::sendControl(const QByteArray &buffer) {
    if (buffer.isEmpty())
        return false;

    quint32 len = 0;
    if (controlSocket)
        len = controlSocket->write(buffer.data(), buffer.length());

    return len == buffer.length() ? true : false;
}

