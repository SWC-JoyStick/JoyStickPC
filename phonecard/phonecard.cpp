#include "phonecard.h"

QPointer<adbProcess> PhoneCard::adb = new adbProcess();

PhoneCard::PhoneCard(QMainWindow *parent): QFrame(parent), cardOnMode(MenuMode::ON_NULL) {}

PhoneCard::PhoneCard(const PhoneCard &phone): phoneInfo(phone.phoneInfo), cardOnMode(phone.cardOnMode) {}

PhoneCard &PhoneCard::operator=(const PhoneCard &phone) {
    this->phoneInfo.deviceName = phone.phoneInfo.deviceName;
    this->phoneInfo.deviceType = phone.phoneInfo.deviceType;
    this->phoneInfo.ip = phone.phoneInfo.ip;
    this->phoneInfo.serial = phone.phoneInfo.serial;
    return *this;
}

bool PhoneCard::operator==(const PhoneCard &phone) const {
    return this->phoneInfo.deviceName == phone.phoneInfo.deviceName;
}

void PhoneCard::setInfo(QString type, QString name, QString ip, QString serial, MenuMode mode) {
    this->phoneInfo.deviceType = type;
    this->phoneInfo.deviceName = name;
    this->phoneInfo.ip = ip;
    this->phoneInfo.serial = serial;
    this->cardOnMode = mode;
}

void PhoneCard::draw(int rank) {
    qDebug() << "PhoneCard::draw";
    qDebug() << "MenuMode: " << this->cardOnMode;
    QString picDir = getenv("MY_IMG_PATH");
    // 根据索引计算
    int number = rank;
    int ax,ay,aw,ah;
    if (number % 2 == 0) {
        // 左边的
        ax = 140;
    } else {
        ax = 490;
    }
    ay = 10 + (number / 2) * 210;
    aw = 300;
    ah = 190;
    this->setGeometry(ax,ay,aw,ah);
    QString devicePicPath = picDir + this->phoneInfo.deviceType;
    this->setStyleSheet(style_one);

    this->drawPicture(devicePicPath);
    this->drawLables();
    this->drawButtons();
    this->show();
}

void PhoneCard::drawButtons() {
    // to phone
    if (this->cardOnMode == MenuMode::ON_PC_TO_ANDROID) {
        // 投到手机
        this->attachToDevice.setParent(this);
        this->attachToDevice.setGeometry(40,110,220,30);
        this->attachToDevice.setText("投屏到此设备");
        this->attachToDevice.setFont(QFont("Times New Roman"));
        this->attachToDevice.setStyleSheet(style_but);
        this->attachToDevice.show();
        // disconnect
        this->detachFromDevice.setParent(this);
        this->detachFromDevice.setGeometry(40,150,220,30);
        this->detachFromDevice.setText("停止投屏");
        this->detachFromDevice.setFont(QFont("Times New Roman"));
        this->detachFromDevice.setStyleSheet(style_but);
        this->detachFromDevice.show();

        this->usb.hide();
        this->wifi.hide();
        // 绑定投屏/结束投屏函数
        this->attachToDevice.disconnect();
        this->detachFromDevice.disconnect();
        connect(&this->attachToDevice, &QPushButton::clicked, this, &PhoneCard::onAttachToDevice);
        connect(&this->detachFromDevice, &QPushButton::clicked, this, &PhoneCard::onDetachFromDevice);

    } else if (this->cardOnMode == MenuMode::ON_ANDROID_TO_PC) {
        // 手机 ===> 电脑
        // wifi按键
        this->wifi.setParent(this);
        this->wifi.setGeometry(40,110,220,30);
        this->wifi.setText("Wifi 连接");
        this->wifi.setFont(QFont("Times New Roman"));
        this->wifi.setStyleSheet(style_but);
        this->wifi.show();
        // USB
        this->usb.setParent(this);
        this->usb.setGeometry(40,150,220,30);
        this->usb.setText("USB 连接");
        this->usb.setFont(QFont("Times New Roman"));
        this->usb.setStyleSheet(style_but);
        this->usb.show();

        this->attachToDevice.hide();
        this->detachFromDevice.hide();
        // 解除绑定
        this->usb.disconnect();
        this->wifi.disconnect();
        // 绑定函数
        connect(&this->usb, &QPushButton::clicked, this, [this]() {
            adb->disConnect(this->phoneInfo.ip, QString::number(ADBPORT));
            adb->disableTcpMode(this->phoneInfo.serial);
            this->onOpenVideoForm(this->phoneInfo.serial);
        });
        connect(&this->wifi, &QPushButton::clicked, this, [this]() {
            adb->Connect(this->phoneInfo.ip, QString::number(ADBPORT));
            adb->enableTcpMode(this->phoneInfo.serial, QString::number(ADBPORT));
            this->onOpenVideoForm(this->phoneInfo.ip + ":" + QString::number(ADBPORT));
        });
    } else {
        this->attachToDevice.hide();
        this->detachFromDevice.hide();
        this->usb.hide();
        this->wifi.hide();
    }

    /**
    connect(this->toAndroid, &ScreenToAndroid::onClosed, this, [this] () {
        qDebug() << "Slot: triggered by onClose()";
        this->onDetachFromDevice();
    });
    */
}

void PhoneCard::drawLables() {
    // 设备名字
    this->deviceLabelName.setParent(this);
    this->deviceLabelName.setGeometry(130,10,140,40);
    this->deviceLabelName.setText(this->phoneInfo.deviceName);
    this->deviceLabelName.show();
    // IP
    this->deviceLabelIP.setParent(this);
    this->deviceLabelIP.setGeometry(130,60,140,40);
    this->deviceLabelIP.setText("IP:"+this->phoneInfo.ip);
    this->deviceLabelIP.show();
}

void PhoneCard::drawPicture(QString devicePicPath) {
    this->deviceLabelPic.setParent(this);
    this->deviceLabelPic.setGeometry(30,10,100,90);
    QPixmap pixdevice(devicePicPath);
    this->deviceLabelPic.setPixmap(pixdevice);
    this->deviceLabelPic.setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->deviceLabelPic.setScaledContents(true);
    this->deviceLabelPic.show();
}

QString PhoneCard::getType() const {
    return phoneInfo.deviceType;
}

QString PhoneCard::getName() const {
    return phoneInfo.deviceName;
}

QString PhoneCard::getIP() const {
    return phoneInfo.ip;
}

QString PhoneCard::getSerial() const {
    return phoneInfo.serial;
}

void PhoneCard::onAttachToDevice() {
    if(this->attachToDevice.text() == "投屏到此设备") {
        this->toAndroid = new ScreenToAndroid(this);
        // 未按下
        this->attachToDevice.setText("投屏中...");
        this->attachToDevice.setFocus();        //按钮2设置为焦点停留状态
        this->attachToDevice.setDisabled(true); //按钮3设置为无法点击状态
        this->attachToDevice.show();
        QMessageBox::information(this,"JoyStick", "投屏已启动!",QMessageBox::Ok);
        // --------------------TODO加投屏函数--------------------------
    }
}

void PhoneCard::onDetachFromDevice() {
    // 点击取消按钮 先判断
    if (this->attachToDevice.text() == "投屏到此设备") {
        QMessageBox::information(this,"JoyStick","你还未开启投屏",QMessageBox::Ok);
        //使 MyBox 对话框显示
        return;
    }
    else {
        this->toAndroid->close();
        this->attachToDevice.setText("投屏到此设备");
        this->attachToDevice.setDisabled(false);
        QMessageBox::information(this,"JoyStick","投屏已停止!",QMessageBox::Ok);
        // --------------------TODO加停止函数--------------------------
    }
}

void PhoneCard::onOpenVideoForm(const QString &serial) {
    videoForm = new VideoForm(serial);
    videoForm->setAttribute(Qt::WA_QuitOnClose, true);
    videoForm->show();
}

void PhoneCard::onCloseVideoForm() {
    if (!this->videoForm.isNull())
        this->videoForm->~VideoForm();
}

