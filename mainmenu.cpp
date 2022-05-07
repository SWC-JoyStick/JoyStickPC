#include "mainmenu.h"
#include "ui_mainmenu.h"

MainMenu::MainMenu(QWidget *parent): QMainWindow(parent), ui(new Ui::MainMenu) {
    ui->setupUi(this);
    this->setMinimumSize(840,630);
    this->setMaximumSize(840,630);

    QString picDir = getenv("MY_IMG_PATH");
    QString iconPic = picDir + "execicon.png";
    // 程序icon
    QIcon icon(iconPic);
    this->setWindowIcon(icon);
    // 左边bar的frame
    frameOfLeftBar.setParent(this);
    frameOfLeftBar.setGeometry(0,0,120,630);
    frameOfLeftBar.setStyleSheet(style_one);
    // 主页button
    mainButton.setParent(&frameOfLeftBar);
    mainButton.setIcon(QPixmap(picDir+"home.png"));
    mainButton.setIconSize(QSize(28,28));
    mainButton.setText("主页");
    mainButton.setStyleSheet(style_one);
    mainButton.setGeometry(10,65,100,80);
    // 投到手机button
    toPhoneButton.setParent(&frameOfLeftBar);
    toPhoneButton.setIcon(QPixmap(picDir+"phone.png"));
    toPhoneButton.setIconSize(QSize(28,28));
    toPhoneButton.setText("投到\n手机");
    toPhoneButton.setStyleSheet(style_one);
    toPhoneButton.setGeometry(10,205,100,80);
    // 投到PC button
    toPcButton.setParent(&frameOfLeftBar);
    toPcButton.setIcon(QPixmap(picDir+"pc.png"));
    toPcButton.setIconSize(QSize(28,28));
    toPcButton.setText("投到\n电脑");
    toPcButton.setStyleSheet(style_one);
    //toPcButton.setFont(QFont("楷体", 10));
    toPcButton.setGeometry(10,345,100,80);

    // 新建一个按键 然后填充上 刷新按键
    refreshButton.setParent(&frameOfLeftBar);
    refreshButton.setGeometry(10,485,100,80);
    refreshButton.setIcon(QPixmap(picDir+"refresh.png"));
    refreshButton.setIconSize(QSize(28,28));
    refreshButton.setText("刷新");
    refreshButton.setStyleSheet(style_one);
    // 主页部分的背景加载在draw里面

    this->bonjour = new Bonjour(this);
    this->menuMode = MenuMode::ON_NULL;
    // 绑定槽函数
    connect(&mainButton, &QPushButton::clicked, this, &MainMenu::onMainButtonClicked);
    // PC mode clicked
    connect(&toPhoneButton, &QPushButton::clicked, this, &MainMenu::onPhoneButtonClicked);
    connect(&toPcButton, &QPushButton::clicked, this, &MainMenu::onPcButtonClicked);
    connect(&refreshButton, &QPushButton::clicked, this, &MainMenu::onRefreshClicked);
    connect(bonjour, &Bonjour::updatePhoneInfo, this, &MainMenu::addUdpDeviceInfo);
    connect(this, &MainMenu::UdpMenuInit, this, &MainMenu::onRefreshClicked);
    connect(this, &MainMenu::AdbMenuInit, this, &MainMenu::onRefreshClicked);
}

MainMenu::~MainMenu() {
    delete ui;
}

void MainMenu::paintEvent(QPaintEvent *event) {
    QString picDir =getenv("MY_IMG_PATH");
    QString bgpath = picDir + "background.jpg";
    QPixmap pix1(bgpath);
    QPixmap temp(pix1.size());
    temp.fill(Qt::transparent);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawPixmap(0,0,width(),height(),QPixmap(bgpath));
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.fillRect(temp.rect(),QColor(0,0,0,222));
    painter.end();
    pix1 = temp;
}

void MainMenu::updateUdpDeviceList() {
    qDebug() << "trigger updateUdpDeviceList";
    // using udp
    int number = 0;
    for (PhoneCard &phone : cardsOfUdpPhone) {
        phone.setParent(this);
        phone.draw(number++);
    }
}

void MainMenu::updateAdbDeviceList() {
    // using adb
    QString deviceType = "other";
    QStringList serials = adb->getDevicesSerial();
    QStringList ips = adb->getDevicesIP(serials);
    QStringList names = adb->getDeviceNames(serials);
    for (int i = 0; i < serials.length(); i++) {
        cardsOfAdbPhone.emplace_back(PhoneCard(this));
        cardsOfAdbPhone.back().setInfo(deviceType, names[i], ips[i], serials[i], this->menuMode);
        cardsOfAdbPhone.back().setParent(this);
        cardsOfAdbPhone.back().draw(i);
    }
}

void MainMenu::addUdpDeviceInfo(QString name, QString ip) {
    // 后续devices、ips等变量是根据udp bonjour过程获取的
    QStringList deviceKeyWords = {"huawei", "iphone", "xiaomi", "oppo", "vivo", "oneplus"};
    // 将新设备存到phoneList中
    QString deviceType = "other";
    for (QString &keyword : deviceKeyWords) {
        if (name.contains(keyword, Qt::CaseInsensitive)) {
           deviceType = keyword;
        }
    }
    PhoneCard phone(this);
    phone.setInfo(deviceType, name, ip, "", this->menuMode);
    if (!cardsOfUdpPhone.contains(phone))
        cardsOfUdpPhone.push_back(phone);
}

void MainMenu::onRefreshClicked() {
    qDebug() << "onRefreshClicked, MenuMode = " << this->menuMode;
    // ----------------TODO: 调用设备发现的接口----------------
    if (this->menuMode == MenuMode::ON_NULL) {
        QMessageBox::information(this,"JoyStick", "请先在左侧选择模式!",QMessageBox::Ok);
        return;
    }

    if (this->menuMode == MenuMode::ON_PC_TO_ANDROID) {
        qDebug() << "投屏到手机...";
        updateUdpDeviceList();
    } else if (this->menuMode == MenuMode::ON_ANDROID_TO_PC) {
        qDebug() << "投屏到电脑...";
        updateAdbDeviceList();
    }
}

void MainMenu::onDisappearClicked() {
    this->setVisible(0);
}

// 只做外观变化 findType 是在发现的时候传过来的 后续不能修改
void MainMenu::onPhoneButtonClicked() {
    QMessageBox::information(this,"JoyStick", "已选择电脑投屏到手机状态!",QMessageBox::Ok);
    this->menuMode = MenuMode::ON_PC_TO_ANDROID;
    // 关闭adb设备发现
    if (!adb.isNull())
        adb->~adbProcess();
    // 启动udp设备发现
    bonjour->start();

    QString picDir = getenv("MY_IMG_PATH");
    toPhoneButton.setIcon(QPixmap(picDir+"run.png"));
    toPhoneButton.setIconSize(QSize(28,28));
    toPhoneButton.setText("已运行");
    toPhoneButton.setFocus();        //按钮2设置为焦点停留状态
    toPhoneButton.setStyleSheet("color: rgb(18,86,199)");
    toPhoneButton.setDisabled(true); //按钮3设置为无法点击状态
    toPhoneButton.show();

    toPcButton.setText("投到\n电脑");
    toPcButton.setDisabled(false);
    toPcButton.setIcon(QPixmap(picDir+"pc.png"));
    toPcButton.setIconSize(QSize(28,28));
    toPcButton.setStyleSheet(style_one);
    toPcButton.show();

    for (PhoneCard &phone : cardsOfAdbPhone) {
        phone.setParent(this);
        phone.hide();
    }
    emit UdpMenuInit();
}

void MainMenu::onPcButtonClicked() {
    QMessageBox::information(this,"JoyStick", "已选择手机投屏到电脑状态!",QMessageBox::Ok);
    this->menuMode = MenuMode::ON_ANDROID_TO_PC;
    // 关闭udp设备发现
    if (!bonjour.isNull())
        bonjour->stop();
    // 开启adb设备发现
    adb = new adbProcess(this);
    QString picDir = getenv("MY_IMG_PATH");
    toPcButton.setIcon(QPixmap(picDir+"run.png"));
    toPcButton.setIconSize(QSize(28,28));
    toPcButton.setText("已运行");
    toPcButton.setFocus();        //按钮2设置为焦点停留状态
    toPcButton.setDisabled(true); //按钮3设置为无法点击状态
    toPcButton.setStyleSheet("color: rgb(18,86,199)");
    toPcButton.show();

    toPhoneButton.setText("投到\n手机");
    toPhoneButton.setDisabled(false);
    toPhoneButton.setIcon(QPixmap(picDir+"phone.png"));
    toPhoneButton.setIconSize(QSize(28,28));
    toPhoneButton.setStyleSheet(style_one);
    toPhoneButton.show();
    for (PhoneCard &phone : cardsOfUdpPhone) {
        phone.setParent(this);
        phone.hide();
    }
    emit AdbMenuInit();
}

void MainMenu::onMainButtonClicked() {
    // 恢复空状态
    this->menuMode = MenuMode::ON_NULL;
    // 关闭udp设备发现
    if (!bonjour.isNull())
        bonjour->~Bonjour();
    // 关闭adb设备发现
    if (!adb.isNull())
        adb->~adbProcess();
    QString picDir = getenv("MY_IMG_PATH");
    toPhoneButton.setText("投到\n手机");
    toPhoneButton.setDisabled(false);
    toPhoneButton.setIcon(QPixmap(picDir+"phone.png"));
    toPhoneButton.setIconSize(QSize(28,28));
    toPhoneButton.setStyleSheet("color: rgb(0,47,167)");
    toPhoneButton.setStyleSheet(style_one);
    toPhoneButton.show();

    toPcButton.setText("投到\n电脑");
    toPcButton.setDisabled(false);
    toPcButton.setIcon(QPixmap(picDir+"pc.png"));
    toPcButton.setIconSize(QSize(28,28));
    toPcButton.setStyleSheet(style_one);
    toPcButton.show();
    for (PhoneCard &phone : cardsOfUdpPhone) {
        phone.setParent(this);
        phone.hide();
    }
    for (PhoneCard &phone : cardsOfAdbPhone) {
        phone.setParent(this);
        phone.hide();
    }
}

