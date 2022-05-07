#include "pccontrolevent.h"

PCControlEvent::PCControlEvent(ControlEventType type): QJoyStickEvent(QJoyStickEvent::RecvControl) {
    data.type = type;
}

PCControlEvent::~PCControlEvent() {

}
