#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMainWindow>
// --------------add new ------------------
#include <QCoreApplication>
#include <QIcon>
#include <QImage>
#include <QFileInfo>
#include <QPainter>

#include "bonjour.h"
#include "phonecard.h"

namespace Ui {
class MainMenu;
}

class MainMenu : public QMainWindow {
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);
    ~MainMenu();


private:
    Ui::MainMenu *ui;

    QPointer<Bonjour> bonjour;              // udp device detector
    QPointer<adbProcess> adb;               // adb device detector
    // 按键三个 主页、历史、待开发
    QPushButton mainButton;
    QPushButton toPhoneButton;
    QPushButton toPcButton;
    QPushButton refreshButton;
    QFrame frameOfLeftBar;
    // 手机的相关信息
    QList<PhoneCard> cardsOfUdpPhone;
    QList<PhoneCard> cardsOfAdbPhone;
    MenuMode menuMode;

    void paintEvent(QPaintEvent *event);
    void updateUdpDeviceList();             // refresh调用刷新udp设备列表
    void updateAdbDeviceList();             // refresh调用刷新adb设备列表

signals:
    void UdpMenuInit();
    void AdbMenuInit();

protected slots:
    void addUdpDeviceInfo(QString name, QString ip);                    // 添加udp新设备
    // 点击刷新 然后 获取设备信息
    void onRefreshClicked();
    //// --------------TODO加slots--------------
    // 三种button wifi连接 对应槽函数什么?
    // USB连接 对应 startserver?
    // 投屏到此设备应该是对应 录屏那个
    // 例如这里我写一个案件消失的slot
    void onDisappearClicked();
    // 点击投屏到手机之后 隐藏 USB 和 WIFI 连接 RESIZE 剩下的俩个button
    // TODO : 数据，需要重新获取数据吗？
    void onPhoneButtonClicked();
    void onPcButtonClicked();
    // 隐藏所有按键
    void onMainButtonClicked();
};

#endif // MAINMENU_H
