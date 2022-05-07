#ifndef SCALER_H
#define SCALER_H

#include <QThread>
#include <QtGlobal>

#include "frameshared.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
}

class Scaler: public QThread {
    Q_OBJECT
public:
    explicit Scaler(FrameShared *pFrameOfScreen, FrameShared *pFrameYUV, AVCodecContext *pCodecCtxOfDecoder, const AVPixelFormat pixelFormat);
    virtual ~Scaler();
    AVPixelFormat getPixelFormat() const;

public slots:
    void stopScaling();

protected:
    void run();

private:
    FrameShared *pFrameOfScreen;
    FrameShared *pFrameYUV;
    /** ========== SWS转码 ===========*/
    struct SwsContext* imgConvertCtx;
    const AVPixelFormat outPixelFormat;
    bool isRunning;

    void scale();
};

#endif // SCALER_H
