#ifndef SCREENDECODER_H
#define SCREENDECODER_H

#include <QThread>
#include <QDebug>
#include "packetshared.h"
#include "frameshared.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

class ScreenDecoder: public QThread {
    Q_OBJECT
public:
    ScreenDecoder(PacketShared *packet, FrameShared *pFrameOfScreen, int videoIndex, AVFormatContext *pFormatCtxOfScreen);
    virtual ~ScreenDecoder();
    AVCodecContext *getCodecCtx() const;

public slots:
    void stopDecoding();

protected:
    void run();

private:
    PacketShared *packet;
    FrameShared *frame;
    /** ========== Decoder ===========*/
    AVCodecContext *pCodecCtxOfDecoder;
    const AVCodec *pCodecOfDecoder;
    bool isRunning;

    void decode();
};

#endif // SCREENDECODER_H
