#ifndef DECODER_H
#define DECODER_H

#include <QThread>
#include <QPointer>
#include "devicesocket.h"
#include "frames.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

class Decoder: public QThread {
    Q_OBJECT
public:
    Decoder();
    virtual ~Decoder();

    static bool init();                         // 初始化ffmpeg
    static void destroy();                      // 退出ffmpeg

    void setFrames(Frames *frames);                         // 保存解码帧
    void setDeviceSocket(DeviceSocket *deviceSocket);       // 设置deviceSocket用于接收视频流数据
    qint32 recvData(quint8 *buf, qint32 bufSize);           // 从deviceSocket接收视频流到buf
    bool startDecode();                         // 开始解码
    void stopDecode();                          // 停止解码

signals:
    void onNewFrame();                          // 有新帧送入渲染器
    void onDecodeStop();                        // 解码停止

protected:
    void run();                                 // QThread开始执行调用run函数
    void pushFrame();                           // 推送帧

private:
    QPointer<DeviceSocket> deviceSocket = Q_NULLPTR;    // 接收视频流数据
    bool isQuit = false;                                // 退出标记
    Frames *frames = Q_NULLPTR;                         // 视频流解码出的帧
};

#endif // DECODER_H
