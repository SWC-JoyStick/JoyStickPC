#ifndef ENCODER_H
#define ENCODER_H

#include <QtGlobal>
#include <QThread>
#include <QDebug>
#include "frameshared.h"
#include "packetshared.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
}


class Encoder: public QThread {
    Q_OBJECT
public:
    Encoder(FrameShared *frame, PacketShared *packet, const AVCodecID codecType, const AVPixelFormat pixelFormat, int inWidth, int inHeight);
    virtual ~Encoder();

public slots:
    void stopEncoding();

protected:
    void run();

private:
    FrameShared *frame;
    PacketShared *packet;
    /** ========== Encoder ===========*/
    AVCodecContext *pCodecCtxOfEncoder;
    const AVCodec *pCodecOfEncoder;
    bool isRunning;

    void encode();
};

#endif // ENCODER_H
