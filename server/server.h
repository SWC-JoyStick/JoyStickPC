#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QSize>
#include <QTcpServer>
#include <QTcpSocket>
#include "adb/adb.h"
#include "devicesocket.h"
#include "tcpserver.h"

/* 使用adb在Android端启动server
 * adb shell "手机存放server的.jar文件的位置" app_process / "server类所在的pakage位置" maxsize: 1080("0"表示Android原生分辨率) bitrate:8000000 adbForward: false clipped_region: ""
 */
class Server: public QObject {
    Q_OBJECT

    enum SERVER_START_STEP {
        SSS_NULL,
        SSS_PUSH,
        SSS_ENABLE_REVERSE,
        SSS_EXECUTE_SERVER,
        SSS_RUNNING,
    };

public:
    Server(QObject *parent = nullptr);
    ~Server();
    bool start(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate);
    void stop();
    DeviceSocket *getDeviceSocket();                              // 对外暴露deviceSocket

signals:
    void serverStartResult(bool success);
    void connectionResult(bool success, const QString &deviceName, const QSize &size);
    void onServerStop();

private slots:
    void onWorkProcessResult(adbProcess::ADB_EXEC_RESULT execResult);

private:
    // functions
    bool startServerByStep();       // server启动状态机
    bool pushServer();              // push server to the android device
    bool removeServer();            // 在安卓设备中删除server
    bool enableTunnelReverse();     // 打开adb reverse反向代理
    bool disableTunnelReverse();    // 关闭adb reverse反向代理
    bool execute();                 // 执行server
    bool startListeningPort();      // 监听端口
    QString getServerPath();        // 获取server在PC端的路径
    bool readInfoFromServer(QString &deviceName, QSize &size);   // 获取android端发过来的deviceName和size
    // variables
    QString serial = "";            // 设备序列号
    quint16 localPort = 27183;      // 端口号
    quint16 maxSize = 1080;          // 分辨率
    quint32 bitRate = 8e6;          // 比特率

    SERVER_START_STEP serverStartStep = SSS_NULL;       // 当前启动状态
    adbProcess workProcess;                             // 用于执行server安装过程的adb指令
    adbProcess serverProcess;                           // adb shell是阻塞的, 因此新建进程去运行
    QString serverPath = "";                            // server文件位于PC端的路径
    bool serverPushedToDevice = false;                  // 是否已经成功将apk推送到android设备
    bool adbReverseEnabled = false;                     // 是否已经开启了adb reverse

    TcpServer serverSocket;                             // PC作为server监听Android的连接
    DeviceSocket *deviceSocket = Q_NULLPTR;             // 连接成功后作为接收视频流/发送控制指令的socket
};

#endif // SERVER_H
