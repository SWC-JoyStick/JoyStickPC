#ifndef PCCONTROLLER_H
#define PCCONTROLLER_H
#include <QObject>
#include <QPointer>
#include <QTcpSocket>

#include "pccontrolevent.h"
#include "Robot.h"

class PCController: public QObject {
    Q_OBJECT
public:
    PCController(QObject *parent = Q_NULLPTR);
    ~PCController();

    void postControlEvent(PCControlEvent *controlEvent);

protected:
    bool event(QEvent *event);

private:
    Robot::Key androidKeycode2pc(AndroidKeycode keycode);
    Robot::Button androidButtons2pc(AndroidMotioneventButtons buttons);
    Robot::Point positionScale(const QRect &position);
};

#endif // PCCONTROLLER_H
