#include "androidconvertnormal.h"
#include "androidcontrolevent.h"

AndroidConvertNormal::AndroidConvertNormal() {

}

AndroidConvertNormal::~AndroidConvertNormal() {

}

void AndroidConvertNormal::mouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize) {
    if (!from)
        return;

    AndroidControlEvent *controlEvent = new AndroidControlEvent(AndroidControlEvent::CET_MOUSE);
    if (!controlEvent)
        return;
    // 转换鼠标动作
    AndroidMotioneventAction action;
    switch (from->type()) {
        case QEvent::MouseButtonPress:
            action = AMOTION_EVENT_ACTION_DOWN;
            break;
        case QEvent::MouseButtonRelease:
            action = AMOTION_EVENT_ACTION_UP;
            break;
        case QEvent::MouseMove:
            action = AMOTION_EVENT_ACTION_MOVE;
            break;
        default:
            break;
    }
    // 转换点按位置pos: position()是事件触发位置与窗口的相对位置, 需要将窗口位置和帧位置进行转换(即窗口大小和帧大小不一定相等)
    // 真实的pos = pos / windowSize * frameSize
    QPointF pos = from->position();
    // 需要注意值不能为负数
    pos.setX(qMax(0.0, pos.x() / showSize.width() * frameSize.width()));
    pos.setY(qMax(0.0, pos.y() / showSize.height() * frameSize.height()));
    // 建立mouse事件
    controlEvent->setMouseEventData(action, qtButtons2android(from->buttons()), QRect(pos.toPoint(), frameSize));
    sendControlEvent(controlEvent);
}

void AndroidConvertNormal::wheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize) {
    if (!from)
        return;

    AndroidControlEvent *controlEvent = new AndroidControlEvent(AndroidControlEvent::CET_SCROLL);
    if (!controlEvent)
        return;
    // 转换点按位置pos: localPos是事件触发位置与窗口的相对位置, 需要将窗口位置和帧位置进行转换(即窗口大小和帧大小不一定相等)
    // 真实的pos = pos / windowSize * frameSize
    QPointF pos = from->position();
    pos.setX(pos.x() / showSize.width() * frameSize.width());
    pos.setY(pos.y() / showSize.height() * frameSize.height());
    // 转换hScroll和vScroll:
    qint32 hScroll = from->angleDelta().x() / 90, vScroll = from->angleDelta().y() / 90;
    // 建立mouse事件
    controlEvent->setScrollEventData(QRect(pos.toPoint(), frameSize), hScroll, vScroll);
    sendControlEvent(controlEvent);
}

void AndroidConvertNormal::keyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize) {
    if (!from)
        return;

    AndroidControlEvent *controlEvent = new AndroidControlEvent(AndroidControlEvent::CET_KEYCODE);
    if (!controlEvent)
        return;
    // 转换键盘动作Press/Release
    AndroidKeyeventAction action;
    switch (from->type()) {
        case QEvent::KeyPress:
            action = AKEY_EVENT_ACTION_DOWN;
            break;
        case QEvent::KeyRelease:
            action = AKEY_EVENT_ACTION_UP;
            break;
        default:
            break;
    }
    // 设置操作的键位keycode
    AndroidKeycode keyCode = qtKeycode2android(from->key(), from->modifiers());
    if (keyCode == AKEYCODE_UNKNOWN)
        return;
    // 建立mouse事件
    controlEvent->setKeycodeEventData(action, keyCode, modifier2metastate(from->modifiers()));
    sendControlEvent(controlEvent);
}

AndroidMotioneventButtons AndroidConvertNormal::qtButtons2android(Qt::MouseButtons buttonState) {
    quint32 buttons = 0;
    if (buttonState & Qt::LeftButton)
        buttons |= AMOTION_EVENT_BUTTON_PRIMARY;
    if (buttonState & Qt::RightButton)
        buttons |= AMOTION_EVENT_BUTTON_SECONDARY;
    if (buttonState & Qt::MiddleButton)
        buttons |= AMOTION_EVENT_BUTTON_TERTIARY;
    if (buttonState & Qt::XButton1)
        buttons |= AMOTION_EVENT_BUTTON_BACK;
    if (buttonState & Qt::XButton2)
        buttons |= AMOTION_EVENT_BUTTON_FORWARD;

    return (AndroidMotioneventButtons)buttons;
}

AndroidKeycode AndroidConvertNormal::qtKeycode2android(qint32 key, Qt::KeyboardModifiers modifiers) {
    AndroidKeycode keyCode = AKEYCODE_UNKNOWN;

    // 功能键
    switch (key) {
        case Qt::Key_Return:
            keyCode = AKEYCODE_ENTER;
            break;
        case Qt::Key_Enter:
            keyCode = AKEYCODE_NUMPAD_ENTER;
            break;
        case Qt::Key_Escape:
            keyCode = AKEYCODE_ESCAPE;
            break;
        case Qt::Key_Backspace:
            keyCode = AKEYCODE_DEL;
            break;
        case Qt::Key_Delete:
            keyCode = AKEYCODE_FORWARD_DEL;
            break;
        case Qt::Key_Tab:
            keyCode = AKEYCODE_TAB;
            break;
        case Qt::Key_Home:
            keyCode = AKEYCODE_MOVE_HOME;
            break;
        case Qt::Key_End:
            keyCode = AKEYCODE_MOVE_END;
            break;
        case Qt::Key_PageUp:
            keyCode = AKEYCODE_PAGE_UP;
            break;
        case Qt::Key_PageDown:
            keyCode = AKEYCODE_PAGE_DOWN;
            break;
        case Qt::Key_Left:
            keyCode = AKEYCODE_DPAD_LEFT;
            break;
        case Qt::Key_Right:
            keyCode = AKEYCODE_DPAD_RIGHT;
            break;
        case Qt::Key_Up:
            keyCode = AKEYCODE_DPAD_UP;
            break;
        case Qt::Key_Down:
            keyCode = AKEYCODE_DPAD_DOWN;
            break;
    }

    if (keyCode != AKEYCODE_UNKNOWN)
        return keyCode;

    // 如果Alt或Meta(就是windows键或control键)按下, 拒绝接收字符和空格
    if (modifiers & (Qt::AltModifier | Qt::MetaModifier))
        return keyCode;

    // 字符键
    switch (key) {
        case Qt::Key_A:
            keyCode = AKEYCODE_A;
            break;
        case Qt::Key_B:
            keyCode = AKEYCODE_B;
            break;
        case Qt::Key_C:
            keyCode = AKEYCODE_C;
            break;
        case Qt::Key_D:
            keyCode = AKEYCODE_D;
            break;
        case Qt::Key_E:
            keyCode = AKEYCODE_E;
            break;
        case Qt::Key_F:
            keyCode = AKEYCODE_F;
            break;
        case Qt::Key_G:
            keyCode = AKEYCODE_G;
            break;
        case Qt::Key_H:
            keyCode = AKEYCODE_H;
            break;
        case Qt::Key_I:
            keyCode = AKEYCODE_I;
            break;
        case Qt::Key_J:
            keyCode = AKEYCODE_J;
            break;
        case Qt::Key_K:
            keyCode = AKEYCODE_K;
            break;
        case Qt::Key_L:
            keyCode = AKEYCODE_L;
            break;
        case Qt::Key_M:
            keyCode = AKEYCODE_M;
            break;
        case Qt::Key_N:
            keyCode = AKEYCODE_N;
            break;
        case Qt::Key_O:
            keyCode = AKEYCODE_O;
            break;
        case Qt::Key_P:
            keyCode = AKEYCODE_P;
            break;
        case Qt::Key_Q:
            keyCode = AKEYCODE_Q;
            break;
        case Qt::Key_R:
            keyCode = AKEYCODE_R;
            break;
        case Qt::Key_S:
            keyCode = AKEYCODE_S;
            break;
        case Qt::Key_T:
            keyCode = AKEYCODE_T;
            break;
        case Qt::Key_U:
            keyCode = AKEYCODE_U;
            break;
        case Qt::Key_V:
            keyCode = AKEYCODE_V;
            break;
        case Qt::Key_W:
            keyCode = AKEYCODE_W;
            break;
        case Qt::Key_X:
            keyCode = AKEYCODE_X;
            break;
        case Qt::Key_Y:
            keyCode = AKEYCODE_Y;
            break;
        case Qt::Key_Z:
            keyCode = AKEYCODE_Z;
            break;
        case Qt::Key_0:
            keyCode = AKEYCODE_0;
            break;
        case Qt::Key_1:
            keyCode = AKEYCODE_1;
            break;
        case Qt::Key_2:
            keyCode = AKEYCODE_2;
            break;
        case Qt::Key_3:
            keyCode = AKEYCODE_3;
            break;
        case Qt::Key_4:
            keyCode = AKEYCODE_4;
            break;
        case Qt::Key_5:
            keyCode = AKEYCODE_5;
            break;
        case Qt::Key_6:
            keyCode = AKEYCODE_6;
            break;
        case Qt::Key_7:
            keyCode = AKEYCODE_7;
            break;
        case Qt::Key_8:
            keyCode = AKEYCODE_8;
            break;
        case Qt::Key_9:
            keyCode = AKEYCODE_9;
            break;
        case Qt::Key_Space:
            keyCode = AKEYCODE_SPACE;
            break;
    }
    return keyCode;
}

AndroidMetastate AndroidConvertNormal::modifier2metastate(Qt::KeyboardModifiers modifiers) {
    int metastate = AMETA_NONE;

    if (modifiers & Qt::ShiftModifier)
        metastate |= AMETA_SHIFT_ON;
    if (modifiers & Qt::ControlModifier)
        metastate |= AMETA_CTRL_ON;
    if (modifiers & Qt::AltModifier)
        metastate |= AMETA_ALT_ON;
    if (modifiers & Qt::MetaModifier)
        metastate |= AMETA_META_ON;
    /*
    if (mod & KMOD_NUM) {
        metastate |= AMETA_NUM_LOCK_ON;
    }
    if (mod & KMOD_CAPS) {
        metastate |= AMETA_CAPS_LOCK_ON;
    }
    if (mod & KMOD_MODE) { // Alt Gr
        // no mapping?
    }
    */
    return (AndroidMetastate)metastate;
}
