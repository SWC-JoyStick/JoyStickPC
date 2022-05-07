#include "frameshared.h"

FrameShared::FrameShared(): empty(1), full(0) {
    frameShared = av_frame_alloc();
}

FrameShared::~FrameShared() {
    av_frame_free(&frameShared);
    frameShared = Q_NULLPTR;
}

void FrameShared::setFrameParams(AVPixelFormat pix_fmt, int width, int height) {
    frameShared->format = pix_fmt;
    frameShared->width = width;
    frameShared->height = height;
}

AVFrame *FrameShared::getFrame() {
    return frameShared;
}
