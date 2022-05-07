#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include "adb.h"

QString adbProcess::adbPath = "";

adbProcess::adbProcess(QObject *parent): QProcess(parent) {
    // 初始化信号
    initSignals();
}

adbProcess::~adbProcess() {
    close();
}

QString adbProcess::getAdbPath() {
    qDebug() << QCoreApplication::applicationDirPath();
    if (adbPath.isEmpty()) {
        adbPath = QString::fromLocal8Bit(qgetenv("MY_ADB_PATH"));
        QFileInfo fileInfo(adbPath);
        qDebug() << adbPath;
        // 防止环境变量未定义该路径
        if (adbPath.isEmpty() || !fileInfo.isFile())
#ifdef Q_OS_WIN32
            adbPath = QCoreApplication::applicationDirPath() + "/adb/adb.exe";
#else
            adbPath = QCoreApplication::applicationDirPath() + "/adb/adb";
#endif
    }

    return adbPath;
}

void adbProcess::execute(const QString &serial, const QStringList &args) {
    QStringList adbArgs;
    if (!serial.isEmpty())  // 多个设备连接, 需要选择
        adbArgs << "-s" << serial;
    adbArgs << args;
    qDebug() << getAdbPath() << adbArgs.join(' ');
    start(getAdbPath(), adbArgs);
}

void adbProcess::push(const QString &serial, const QString &localPath, const QString &remotePath) {
    QStringList adbArgs = {"push", localPath, remotePath};
    execute(serial, adbArgs);
}

void adbProcess::remove(const QString &serial, const QString &path) {
    QStringList adbArgs = {"shell", "rm", path};
    execute(serial, adbArgs);
}

void adbProcess::adbForward(const QString &serial, quint16 localPort, const QString &deviceSocketName) {
    QStringList adbArgs = {"forward", QString("tcp:%1").arg(localPort), QString("localabstract:%1").arg(deviceSocketName)};
    execute(serial, adbArgs);
}

void adbProcess::adbForwardClear(const QString &serial, quint16 localPort) {
    QStringList adbArgs = {"forward", "--remove", QString("tcp:%1").arg(localPort)};
    execute(serial, adbArgs);
}

void adbProcess::adbReverse(const QString &serial, const QString &deviceSocketName, quint16 localPort) {
    QStringList adbArgs = {"reverse", QString("localabstract:%1").arg(deviceSocketName), QString("tcp:%1").arg(localPort)};
    execute(serial, adbArgs);
}

void adbProcess::adbReverseClear(const QString &serial, const QString &deviceSocketName) {
    QStringList adbArgs = {"reverse", "--remove", QString("localabstract:%1").arg(deviceSocketName)};
    execute(serial, adbArgs);
}

void adbProcess::install(const QString &serial, const QString &localAPK) {
    QStringList adbArgs = {"install", "-r", localAPK};
    execute(serial, adbArgs);
}

void adbProcess::disableTcpMode(const QString &serial) {
    QStringList adbArgs = {"usb"};
    execute(serial, adbArgs);
    waitForFinished();
}

void adbProcess::enableTcpMode(const QString &serial, const QString &port) {
    QStringList adbArgs = {"tcpip", port};
    execute(serial, adbArgs);
    waitForFinished();
}

void adbProcess::Connect(const QString &ip, const QString &port) {
    QStringList adbArgs = {"connect", ip + ":" + port};
    execute("", adbArgs);
}

void adbProcess::disConnect(const QString &ip, const QString &port) {
    QStringList adbArgs = {"disconnect"};
    if (!ip.isEmpty() && !port.isEmpty())
        adbArgs << ip + ":" + port;
    execute("", adbArgs);
}

QStringList adbProcess::getDevicesSerial() {
    /* adb devices
     * List of devices attached
     * emulator-5554	device
     *
     * 是否要单独运行这个指令, 自动获取ip地址呢? 讨论一下;
     */
    QStringList adbArgs = {"devices"};
#ifdef QT_DEBUG
    qDebug() << adbArgs;
#endif
    start(getAdbPath(), adbArgs);
    waitForFinished();
    return getDevicesSerialFromStdout();
}

QStringList adbProcess::getDevicesSerialFromStdout() {
    /* adb devices
     * List of devices attached
     * emulator-5554	device
     *
     * 是否要单独运行这个指令, 自动获取ip地址呢? 讨论一下;
     * QStringList adbArgs = "devices";
     * start(getAdbPath(), adbArgs);
     */
    QStringList serials;
    // 分割adb的输出, 提取设备信息为list
    QStringList devicesInfoList = standardOutput.split(QRegularExpression("\r\n|\n"), Qt::SkipEmptyParts);
    // 从list中提取序列号
    for (const QString &deviceInfo : devicesInfoList) {
        QStringList deviceInfoAll = deviceInfo.split(QRegularExpression("\t"), Qt::SkipEmptyParts);
        if (deviceInfoAll.count() == 2 && deviceInfoAll[1].compare("device") == 0)
            serials << deviceInfoAll[0];
    }

    return serials;
}

QStringList adbProcess::getDevicesIP(const QStringList &serials) {
    /* adb shell ip route | awk '{print $9}'
     * 输出每一行为设备的ip
     * 是否要单独运行这个指令, 自动获取ip地址呢? 讨论一下;
     */
    QStringList ips;
    for (const QString &serial : serials) {
        QStringList adbArgs = {"-s", serial};
        adbArgs << QString("shell ip route | awk").split(' ') << "'{print $9}'";
        qDebug() << adbArgs;
        start(getAdbPath(), adbArgs);
        waitForFinished();
        ips.append(standardOutput.split('\n'));
    }
    return ips;
}

QString adbProcess::getDeviceIPFromStdout() {
    /* adb shell ip -f inet addr show wlan0
     * 15: wlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
     * inet 10.0.2.16/24 brd 10.0.2.255 scope global wlan0
        * valid_lft forever preferred_lft forever
     * 通过正则表达式提取ip地址
     *
     * 是否要单独运行这个指令, 自动获取ip地址呢? 讨论一下;
     * QStringList adbArgs = QString("adb shell ip -f inet addr show wlan0").split(" ");
     * start(getAdbPath(), adbArgs);
     */
    QString ip;
    QString strIPExp = "inet [\\d.]*";
    QRegularExpression ipRegExp(strIPExp, QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpressionMatch match = ipRegExp.match(standardOutput);
    if (match.hasMatch()) {
        ip = match.captured(0);
        ip = ip.mid(5);
    }

    return ip;
}

QStringList adbProcess::getDeviceNames(const QStringList &serials) {
    /* adb -s {serial} shell getprop ro.vendor.oplus.market.name ---- 每个手机不一样, 因此采用设备名进行标识
     * adb -s {serial} shell getprop ro.product.name
     * 输出每一行为设备的设备名
     */
    QStringList names;
    for (const QString &serial : serials) {
        QStringList adbArgs = {"-s", serial};
        adbArgs << QString("shell getprop ro.product.name").split(' ');
        qDebug() << adbArgs;
        start(getAdbPath(), adbArgs);
        waitForFinished();
        names.append(standardOutput.split('\n'));
    }
    return names;
}

QString adbProcess::getStdout() {
    return standardOutput;
}

QString adbProcess::getErrorout() {
    return errorOutput;
}

void adbProcess::initSignals() {
    // 连接信号与槽
    // catch errors, 发出对应的信号
    connect(this, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) { /* */
        if (error == FailedToStart)
            emit adbExecResult(ADB_ERROR_MISSING_BINARY);
        else if (error == Crashed) {
#ifdef QT_DEBUG
            qDebug() << this->processId() << " Crashed.";
#endif
        } else
            emit adbExecResult(ADB_ERROR_START);
        qDebug() << error;
    });
    connect(this, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) { /* */
        if (exitStatus == QProcess::NormalExit && exitCode == 0)
            emit adbExecResult(ADB_SUCCESS_EXEC);
        else
            emit adbExecResult(ADB_ERROR_EXEC);
        qDebug() << exitCode << exitStatus;
    });
    connect(this, &QProcess::readyReadStandardError, this, [this]() { /* */
        errorOutput = QString::fromLocal8Bit(this->readAllStandardError()).trimmed();
        qDebug() << errorOutput;
    });
    connect(this, &QProcess::readyReadStandardOutput, this, [this]() { /* */
        standardOutput = QString::fromLocal8Bit(this->readAllStandardOutput()).trimmed();
        qDebug() << standardOutput;
    });
    connect(this, &QProcess::started, this, [this]() { /* */
        emit adbExecResult(ADB_SUCCESS_START);
    });
    connect(this, &QProcess::stateChanged, this, [this](QProcess::ProcessState state) { /* */
#ifdef QT_DEBUG
        qDebug() << this->processId() << " state changed to " << state;
#endif
    });
}
