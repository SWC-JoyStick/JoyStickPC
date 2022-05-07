QT       += core gui network openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = JoyStick

ICON = icon.ico

RC_ICONS = icon.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainmenu.cpp \
    videoform.cpp

HEADERS += \
    mainmenu.h \
    videoform.h

FORMS += \
    mainmenu.ui \
    videoform.ui

RESOURCES += \
    images.qrc

TRANSLATIONS += \
    QtJoyStick_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# adb module
include($$PWD/adb/adb.pri)
# android server module
include($$PWD/server/server.pri)
# frames module
include($$PWD/frames/frames.pri)
# decoder module
include($$PWD/decoder/decoder.pri)
# common module
include($$PWD/common/common.pri)
# render module
include($$PWD/render/render.pri)
# android module
include($$PWD/android/android.pri)
# controller module
include($$PWD/controller/controller.pri)
# pc2android module
include($$PWD/screen2android/screen2android.pri)
# bonjour: detect each other
include($$PWD/bonjour/bonjour.pri)
# capturer module
include($$PWD/capturer/capturer.pri)
# screen decoder module
include($$PWD/screendecoder/screendecoder.pri)
# scaler module
include($$PWD/scaler/scaler.pri)
# encoder module
include($$PWD/encoder/encoder.pri)
# sender module
include($$PWD/sender/sender.pri)
# shared frame module
include($$PWD/frame/frame.pri)
# shared packet module
include($$PWD/packet/packet.pri)
# phone info card module
include($$PWD/phonecard/phonecard.pri)

win32 {
# .dll
LIBS += -L$$PWD/ffmpeg/win/bin \

# dll.a
LIBS += -L$$PWD/ffmpeg/win/lib \
        -L$$PWD/robot/win/ \
}

macos {
LIBS += \
    -L$$PWD/ffmpeg/mac/lib \
    -L$$PWD/robot/mac/ \
}

linux {

}

# include path
INCLUDEPATH += \
    $$PWD/ffmpeg/include \
    $$PWD/robot/include \
    $$PWD/adb \
    $$PWD/server \
    $$PWD/frames \
    $$PWD/encoder \
    $$PWD/decoder \
    $$PWD/common \
    $$PWD/render \
    $$PWD/android \
    $$PWD/controller \
    $$PWD/screen2android \
    $$PWD/bonjour \
    $$PWD/capturer \
    $$PWD/screendecoder \
    $$PWD/scaler \
    $$PWD/encoder \
    $$PWD/frame \
    $$PWD/packet \
    $$PWD/sender \
    $$PWD/phonecard \

# libs path
LIBS += -lavformat \
        -lavcodec \
        -lavutil \
        -lswscale \
        -lavdevice \
        -lRobot \
