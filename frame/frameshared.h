#ifndef FRAMESHARED_H
#define FRAMESHARED_H

#include <QCoreApplication>
#include <QSemaphore>


extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

class FrameShared {
public:
    QSemaphore empty;
    QSemaphore full;

    FrameShared();
    ~FrameShared();
    void setFrameParams(AVPixelFormat pix_fmt, int width, int height);
    AVFrame *getFrame();

private:
    AVFrame *frameShared;
};

#endif // FRAMESHARED_H
