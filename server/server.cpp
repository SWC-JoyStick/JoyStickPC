#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>

#include "server.h"

#define DEVICE_SERVER_PATH "/data/local/tmp/scrcpy-server.jar"
#define SOCKET_NAME "scrcpy"
#define DEVICE_NAME_FIELD_LENGTH 64

Server::Server(QObject *parent): QObject(parent) {
    connect(&workProcess, &adbProcess::adbExecResult, this, &Server::onWorkProcessResult);     // workProcess处理, 用于预启动android端的server, 执行push、reverse等
    connect(&serverProcess, &adbProcess::adbExecResult, this, &Server::onWorkProcessResult);   // serverProcess处理, 用于启动android端server, 执行execute
    connect(&serverSocket, &QTcpServer::newConnection, this, [this](){                              // socket连接处理
        deviceSocket = dynamic_cast<DeviceSocket*> (serverSocket.nextPendingConnection());

        QString deviceName;
        QSize size;
        // device name & size会在连接成功后发回PC端
        if (deviceSocket && deviceSocket->isValid() && readInfoFromServer(deviceName, size)) {
            disableTunnelReverse();
            removeServer();
            emit connectionResult(true, deviceName, size);
        } else {
            stop();
            emit connectionResult(false, deviceName, size);
        }
    });
}

Server::~Server() {

}

bool Server::start(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate) {
    // 保存输入状态
    this->serial = serial;
    this->localPort = localPort;
    this->maxSize = maxSize;
    this->bitRate = bitRate;
    // 状态机编程: step1: 设置状态, step2: 启动状态机执行对应状态的操作
    // push server to the android device
    serverStartStep = SSS_PUSH;

    return startServerByStep();
}

void Server::stop() {
    if (deviceSocket)
        deviceSocket->close();
    serverProcess.kill();
    disableTunnelReverse();
    removeServer();
    serverSocket.close();
}

void Server::onWorkProcessResult(adbProcess::ADB_EXEC_RESULT execResult) {

    if (sender() == &workProcess) {
        if (serverStartStep != SSS_NULL) {
            switch (serverStartStep) {
                case SSS_PUSH:
                    // 成功状态: ADB_SUCCESS_EXEC || ADB_SUCCESS_START
                    if (execResult == adbProcess::ADB_SUCCESS_EXEC) {           // push执行成功
                        serverPushedToDevice = true;
                        serverStartStep = SSS_ENABLE_REVERSE;
                        startServerByStep();
                    } else if (execResult != adbProcess::ADB_SUCCESS_START) {   // push执行失败
                        qCritical("adb push failed.");      // 与qDebug一样, 属于log
                        serverStartStep = SSS_NULL;
                        emit serverStartResult(false);
                    }
                    break;
                case SSS_ENABLE_REVERSE:
                    // 成功状态: ADB_SUCCESS_EXEC || ADB_SUCCESS_START
                    if (execResult == adbProcess::ADB_SUCCESS_EXEC) {           // reverse执行成功
                        adbReverseEnabled = true;
                        serverStartStep = SSS_EXECUTE_SERVER;
                        startServerByStep();
                    } else if (execResult != adbProcess::ADB_SUCCESS_START) {   // reverse执行失败
                        qCritical("adb reverse failed.");      // 与qDebug一样, 属于log
                        serverStartStep = SSS_NULL;
                        // server已经被push上去了, 需要删除server
                        removeServer();
                        emit serverStartResult(false);
                    }
                    break;
                default:
                    break;
            }
        }
    } else if (sender() == &serverProcess) {
        if (serverStartStep == SSS_EXECUTE_SERVER) {
            if (execResult == adbProcess::ADB_SUCCESS_START) {
                serverStartStep = SSS_RUNNING;
                emit serverStartResult(true);
            } else if (execResult == adbProcess::ADB_ERROR_START) {
                // server启动失败
                qCritical("adb shell start server failed.");
                serverStartStep = SSS_NULL;
                // 关闭adb reverse
                // 删除server
                disableTunnelReverse();
                removeServer();
                emit serverStartResult(false);
            }
        } else if (serverStartStep == SSS_RUNNING) {
            serverStartStep = SSS_NULL;
            emit onServerStop();
        }
    }
}

bool Server::startServerByStep() {
    bool stepSuccess = false;
    // push, enable_reverse, execute_server
    if (serverStartStep != SSS_NULL) {
        switch (serverStartStep) {
            case SSS_PUSH:
                stepSuccess = pushServer();
                break;
            case SSS_ENABLE_REVERSE:
                stepSuccess = enableTunnelReverse();
                break;
            case SSS_EXECUTE_SERVER:
                if (!startListeningPort())
                    return false;
                stepSuccess = execute();
                break;
            default:
                break;
        }
    }

    return stepSuccess;
}

bool Server::pushServer() {
    workProcess.push(serial, getServerPath(), DEVICE_SERVER_PATH);
    return true;
}

bool Server::removeServer() {
    if (!serverPushedToDevice)
        return true;

    adbProcess *adb = new adbProcess();
    if (!adb)
        return false;
    // 连接信号, 将new出来的adb删除掉 ---- 未启动成功
    connect(adb, &adbProcess::adbExecResult, this, [this](adbProcess::ADB_EXEC_RESULT execResult) {
        if (execResult != adbProcess::ADB_SUCCESS_START)
            sender()->deleteLater();            // 自动删除信号发送方
    });
    // adb启动成功, 需要执行操作后释放
    adb->remove(serial, DEVICE_SERVER_PATH);
    serverPushedToDevice = false;
    return true;
}

bool Server::enableTunnelReverse() {
    workProcess.adbReverse(serial, SOCKET_NAME, localPort);
    return true;
}

bool Server::disableTunnelReverse() {
    if (!adbReverseEnabled)
        return true;

    adbProcess *adb = new adbProcess();
    if (!adb)
        return false;
    // 连接信号, 将new出来的adb删除掉 ---- 未启动成功
    connect(adb, &adbProcess::adbExecResult, this, [this](adbProcess::ADB_EXEC_RESULT execResult) {
        if (execResult != adbProcess::ADB_SUCCESS_START)
            sender()->deleteLater();            // 自动删除信号发送方
    });
    // adb启动成功, 需要执行操作后释放
    adb->adbReverseClear(serial, SOCKET_NAME);
    adbReverseEnabled = false;
    return true;
}

bool Server::execute() {
    /* 使用adb在Android端启动server
     * adb shell "手机存放server的.jar文件的位置" app_process / "server类所在的pakage位置" maxsize: 1080("0"表示Android原生分辨率) bitrate:8000000 adbForward: false clipped_region: ""
     */
    QStringList serverArgs = {"shell", QString("CLASSPATH=%1").arg(DEVICE_SERVER_PATH), "app_process", "/", "com.genymobile.scrcpy.Server"};
    serverArgs << QString::number(maxSize);
    serverArgs << QString::number(bitRate);
    serverArgs << "false";
    serverArgs << "";
    serverProcess.execute(serial, serverArgs);
    return true;
}

bool Server::startListeningPort() {
    serverSocket.setMaxPendingConnections(1);       // 设置最大连接数
    if (!serverSocket.listen(QHostAddress::LocalHost, localPort)) {     // 监听本机localPort端口, 阻塞
        qCritical() << QString("Could not listen on port %1").arg(localPort);
        serverStartStep = SSS_NULL;
        disableTunnelReverse();
        removeServer();
        emit serverStartResult(false);
        return false;
    }
    return true;
}

QString Server::getServerPath() {
    if (serverPath.isEmpty()) {
        serverPath = QString::fromLocal8Bit(qgetenv("MY_SERVER_PATH"));
        QFileInfo fileInfo(serverPath);
        // 防止环境变量未定义该路径
        if (serverPath.isEmpty() || !fileInfo.isFile())
            serverPath = QCoreApplication::applicationDirPath() + "/server/scrcpy-server.jar";
    }

    return serverPath;
}

DeviceSocket *Server::getDeviceSocket() {
    return this->deviceSocket;
}

bool Server::readInfoFromServer(QString &deviceName, QSize &size) {
    /* abk001-------------------------0x0438 0x02d0
     *                  64b           2b w   2b h
     * deviceName: 64字节, size: 2字节宽, 2字节高
     */
    unsigned char buffer[DEVICE_NAME_FIELD_LENGTH + 4];
    // 收到了不足64 + 4字节, 等待300ms
    if (deviceSocket->bytesAvailable() <= (DEVICE_NAME_FIELD_LENGTH + 4))
        deviceSocket->waitForReadyRead(300);

    qint64 len = deviceSocket->read((char *)buffer, sizeof(buffer));
    if (len < DEVICE_NAME_FIELD_LENGTH + 4) {
        qInfo("Could not retrieve device information.");
        return false;
    }
    buffer[DEVICE_NAME_FIELD_LENGTH - 1] = '\0';
    deviceName = (char *)buffer;
    // 0x0000
    // 0x0400
    // 0x0038
    // 0x0438
    // buffer中以字节存储, 左移8位将宽度/高度的最高位获得, 然后与最低位 与运算, 强制转化为int即可
    size.setWidth((buffer[DEVICE_NAME_FIELD_LENGTH] << 8) | buffer[DEVICE_NAME_FIELD_LENGTH + 1]);
    size.setHeight((buffer[DEVICE_NAME_FIELD_LENGTH + 2] << 8) | buffer[DEVICE_NAME_FIELD_LENGTH + 3]);
    return true;
}
