#ifndef SCREENTOANDROID_H
#define SCREENTOANDROID_H

#include <QtGlobal>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
}

#include "frameshared.h"
#include "packetshared.h"
#include "capturer.h"
#include "screendecoder.h"
#include "scaler.h"
#include "encoder.h"
#include "sender.h"
#include "pcconverter.h"

class ScreenToAndroid: public QObject {
    Q_OBJECT
public:
    ScreenToAndroid(QObject *parent = nullptr);
    void close();

signals:
    void onClosed();

private:
    QPointer<Capturer> capturer;
    QPointer<ScreenDecoder> decoder;
    QPointer<Scaler> scaler;
    QPointer<Encoder> encoder;
    QPointer<Sender> sender;
    QPointer<PCConverter> converter;
    // 录屏 -- 解码 -- 转码 -- 编码过程相关共享变量
    PacketShared packetOfScreen;
    FrameShared pFrameOfScreen;
    FrameShared pFrameYUV;
    PacketShared packetOfSocket;
};

#endif // SCREENTOANDROID_H
