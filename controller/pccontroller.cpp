#include <QCoreApplication>
#include "pccontroller.h"

PCController::PCController(QObject *parent): QObject(parent) {

}

PCController::~PCController() {

}

void PCController::postControlEvent(PCControlEvent *controlEvent) {
    if (controlEvent)
        QCoreApplication::postEvent(this, controlEvent);
}

bool PCController::event(QEvent *event) {
    if (event && event->type() == PCControlEvent::RecvControl) {
        PCControlEvent *controlEvent = dynamic_cast<PCControlEvent *>(event);
        // 取消执行延迟
        Robot::Keyboard keyboard;
        keyboard.AutoDelay.Min = -1;
        keyboard.AutoDelay.Max = -1;
        Robot::Mouse mouse;
        mouse.AutoDelay.Min = -1;
        mouse.AutoDelay.Max = -1;
        // 初始化相关临时变量
        QRect position;
        AndroidKeyeventAction keyAction;
        AndroidKeycode keycode;
        AndroidMotioneventAction mouseAction;
        AndroidMotioneventButtons buttons;

        switch (controlEvent->data.type) {
            case CET_KEYCODE:
                keyAction = controlEvent->data.keycodeEvent.action;
                keycode = controlEvent->data.keycodeEvent.keycode;
                if (keyAction == AMOTION_EVENT_ACTION_DOWN)
                    keyboard.Press(androidKeycode2pc(keycode));
                else if (keyAction == AMOTION_EVENT_ACTION_UP)
                    keyboard.Release(androidKeycode2pc(keycode));
                break;
            case CET_TEXT:
                break;
            case CET_MOUSE:
                mouseAction = controlEvent->data.mouseEvent.action;
                buttons = controlEvent->data.mouseEvent.buttons;
                switch (mouseAction) {
                    case AMOTION_EVENT_ACTION_DOWN:
                        mouse.Press(androidButtons2pc(buttons));
                        break;
                    case AMOTION_EVENT_ACTION_UP:
                        mouse.Release(androidButtons2pc(buttons));
                        break;
                    case AMOTION_EVENT_ACTION_MOVE:
                        break;
                    default:
                        break;
                }
                break;
            case CET_SCROLL:
                break;
            case CET_COMMAND:
                break;
            case CET_TOUCH:
                position = controlEvent->data.touchEvent.position;
                mouse.SetPos(positionScale(position));
                mouse.Click(Robot::ButtonLeft);
                break;
            default:
                break;
        }
    }
}

Robot::Key PCController::androidKeycode2pc(AndroidKeycode keycode) {
    Robot::Key pcKeycode = Robot::KeyShift;

    switch (keycode) {
        case AKEYCODE_W:
            pcKeycode = Robot::KeyW;
            break;
        case AKEYCODE_A:
            pcKeycode = Robot::KeyA;
            break;
        case AKEYCODE_S:
            pcKeycode = Robot::KeyS;
            break;
        case AKEYCODE_D:
            pcKeycode = Robot::KeyD;
            break;
        case AKEYCODE_DPAD_UP:
            pcKeycode = Robot::KeyUp;
            break;
        case AKEYCODE_DPAD_LEFT:
            pcKeycode = Robot::KeyLeft;
            break;
        case AKEYCODE_DPAD_DOWN:
            pcKeycode = Robot::KeyDown;
            break;
        case AKEYCODE_DPAD_RIGHT:
            pcKeycode = Robot::KeyRight;
            break;
        default:
            break;
    }
    return pcKeycode;
}

Robot::Button PCController::androidButtons2pc(AndroidMotioneventButtons buttons) {
    Robot::Button mouseButton = Robot::ButtonX2;

    switch (buttons) {
        case AMOTION_EVENT_BUTTON_PRIMARY:
            mouseButton = Robot::ButtonLeft;
            break;
        case AMOTION_EVENT_BUTTON_SECONDARY:
            mouseButton = Robot::ButtonRight;
            break;
        case AMOTION_EVENT_BUTTON_TERTIARY:
            mouseButton = Robot::ButtonMiddle;
            break;
        default:
            break;
    }
    return mouseButton;
}

Robot::Point PCController::positionScale(const QRect &position) {
    Robot::Point scaledPosition(position.x(), position.y());
    return scaledPosition;
}
