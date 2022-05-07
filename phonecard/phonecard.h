#ifndef PHONECARD_H
#define PHONECARD_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QMessageBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>

#include "videoform.h"
#include "screentoandroid.h"
#include "adb.h"

// QSS: Button style and Frame style
static QString style_one = "*{background:rgba(255, 255, 255,0.6);\
        font-size:24px;font-style:MingLiU-ExtB;}\
        QPushButton{background:rgb(255, 255, 255);\
        border:0px;border-style:outset;\
        border-radius:16px;\
        font-style:MingLiU-ExtB;\
        }\
        QPushButton:hover\
        {\
          color:white;\
          background-color:rgba(44 , 137 , 255,0.8);\
          border: 1px solid rgb(11 , 137 , 234);\
        }\
        QPushButton:pressed{\
        background-color:rgb(45,21,34,80);\
        border-style:inset;\
        font-style:MingLiU-ExtB;\
        }\
        QCheckBox{\
        background:rgba(85,170,255,0);\
        color:white;\
        font-style:MingLiU-ExtB;\
        font-family:Times New Roman;\
        }\
        QLabel{\
        background:rgba(85,170,255,0);\
        color:black;\
        font-family:Times New Roman;\
        font-style:MingLiU-ExtB;\
        font-size:18px;\
        }";

static QString style_but = "*{background:rgba(255, 255, 255,0.6);\
        font-size:18px;font-style:MingLiU-ExtB;}\
        QPushButton{background:rgb(255, 255, 255);\
        border:0px;border-style:outset;\
        border-radius:16px;\
        font-style:MingLiU-ExtB;\
        }\
        QPushButton:hover\
        {\
          color:white;\
          background-color:rgba(44 , 137 , 255,0.8);\
          border: 1px solid rgb(11 , 137 , 234);\
        }\
        QPushButton:pressed{\
        background-color:rgb(45,21,34,80);\
        border-style:inset;\
        font-style:MingLiU-ExtB;\
        }";


// 当前窗口状态(PC mode / Android mode)
enum MenuMode {
    ON_NULL,
    ON_PC_TO_ANDROID,
    ON_ANDROID_TO_PC
};

class PhoneCard: public QFrame {
    Q_OBJECT
public:
    PhoneCard(QMainWindow *parent = nullptr);
    PhoneCard(const PhoneCard &phone);
    PhoneCard &operator=(const PhoneCard &phone);

    // 设置手机信息
    bool operator==(const PhoneCard &phone) const;

    void setInfo(QString type="", QString name="", QString ip="", QString serial="", MenuMode mode = MenuMode::ON_NULL);
    void draw(int rank = 0);
    void drawButtons();
    void drawLables();
    void drawPicture(QString devicePicPath);
    QString getType() const;
    QString getName() const;
    QString getIP() const;
    QString getSerial() const;

private:
    // 手机的相关信息
    struct PhoneInfo {
        /** 手机型号
        enum PhoneType {
            HUAWEI,
            IPHONE,
            XIAOMI,
            OPPO,
            VIVO,
            ONEPLUS,
            OTHERS
        };
        */
        QString deviceType;
        QString deviceName;     // 名称
        QString ip;             // ip
        QString serial;         // 序列号 --- 用于usb连接

        PhoneInfo(QString type="", QString name="", QString ip="", QString serial="")
            : deviceType(type), deviceName(name), ip(ip), serial(serial) {}
        PhoneInfo(const PhoneInfo &info)
            : deviceType(info.deviceType), deviceName(info.deviceName), ip(info.ip), serial(info.serial) {}
    };

    QLabel deviceLabelPic;                      // 设备图片
    QLabel deviceLabelName;                     // 设备名称
    QLabel deviceLabelIP;                       // 设备IP
    QPushButton wifi;
    QPushButton usb;
    QPushButton attachToDevice;
    QPushButton detachFromDevice;
    PhoneInfo phoneInfo;                        // 手机设备信息
    MenuMode cardOnMode;                        // 当前窗口模式, 根据这个模式决定卡片展示信息

    QPointer<VideoForm> videoForm;              // android2pc
    QPointer<ScreenToAndroid> toAndroid;        // pc2android
    static QPointer<adbProcess> adb;

private slots:
    void onAttachToDevice();
    void onDetachFromDevice();
    void onOpenVideoForm(const QString &serial);
    void onCloseVideoForm();
};

#endif // PHONECARD_H
