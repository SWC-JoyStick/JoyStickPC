#ifndef ADB_H
#define ADB_H

#include <QProcess>
#define ADBPORT 5555

class adbProcess: public QProcess {
    Q_OBJECT
public:
    enum ADB_EXEC_RESULT {   // 定义adb运行状态, 对状态进行处理
        ADB_SUCCESS_START,
        ADB_ERROR_START,
        ADB_SUCCESS_EXEC,
        ADB_ERROR_EXEC,
        ADB_ERROR_MISSING_BINARY,
    };

    adbProcess(QObject *parent = nullptr);
    ~adbProcess();

    QString getAdbPath();                                                                       // 获取PC端adb可执行文件所在路径
    void execute(const QString &serial, const QStringList &args);                               // ADB: 设备序列号 + 运行参数
    void push(const QString &serial, const QString &localPath, const QString &remotePath);      // 将PC中localPath的文件推送到序列号为serial的设备的remotePath中
    void remove(const QString &serial, const QString &path);                                    // 删除序列号为serial的设备path位置的文件
    void adbForward(const QString &serial, quint16 localPort, const QString &deviceSocketName); // 将PC端口映射到Android, 实现数据转发
    void adbForwardClear(const QString &serial, quint16 localPort);                             // 删除forward端口映射
    void adbReverse(const QString &serial, const QString &deviceSocketName, quint16 localPort); // 将Android端口映射到PC，实现数据转发
    void adbReverseClear(const QString &serial, const QString &deviceSocketName);               // 删除reverse端口映射
    void install(const QString &serial, const QString &localAPK);                               // 将本地的apk安装到手机
    void disableTcpMode(const QString &serial);                                                 // 关闭TCP/IP连接模式
    void enableTcpMode(const QString &serial, const QString &port);                             // 开启TCP/IP连接模式
    void Connect(const QString &ip, const QString &port);                                       // 连接设备, 开启TCP/IP前置条件
    void disConnect(const QString &ip, const QString &port);                                    // 断开连接, 返回usb模式
    QStringList getDevicesSerial();                                                             // 独立获取所有设备序列号
    QStringList getDevicesSerialFromStdout();                                                   // 从adb输出中获取设备序列号
    QStringList getDevicesIP(const QStringList &serials);                                       // 独立获取所有设备ip
    QString getDeviceIPFromStdout();                                                            // 从adb输出中获取设备ip地址
    QStringList getDeviceNames(const QStringList &serials);                                     // 从adb输出中获取所有设备商业名
    QString getStdout();
    QString getErrorout();

signals:
    void adbExecResult(adbProcess::ADB_EXEC_RESULT execResult);

private:
    void initSignals();

    static QString adbPath;
    QString standardOutput = "";
    QString errorOutput = "";
};

#endif // ADB_H
