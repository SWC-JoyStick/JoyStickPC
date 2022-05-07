#include "scaler.h"

Scaler::Scaler(FrameShared *pFrameOfScreen, FrameShared *pFrameYUV, AVCodecContext *pCodecCtxOfDecoder, const AVPixelFormat pixelFormat)
        : outPixelFormat(pixelFormat) {
    /** ==============像素信息空间格式转换过程: 录屏像素 --> YUV420P 创建上下文, 分配内存空间 ================= */
    imgConvertCtx = sws_getContext(
        pCodecCtxOfDecoder->width,
        pCodecCtxOfDecoder->height,
        pCodecCtxOfDecoder->pix_fmt,
        pCodecCtxOfDecoder->width,
        pCodecCtxOfDecoder->height,
        this->outPixelFormat, SWS_BICUBIC, NULL, NULL, NULL
    );
    /**
     * 屏幕像素数组: 分配一块内存空间
     * 该空间大小 = AV_PIX_FMT_YUV420P格式的像素数据在原录屏大小 (width, height) 所占内存大小
     */
    quint8* screenBuffer = (quint8*)av_malloc(
        av_image_get_buffer_size(
            this->outPixelFormat,
            pCodecCtxOfDecoder->width,
            pCodecCtxOfDecoder->height,
            1
        )
    );
    // 分配的该内存区域分给pFrameYUV, 以便于存放后续转换的YUV420P像素数据
    av_image_fill_arrays(
        pFrameYUV->getFrame()->data,
        pFrameYUV->getFrame()->linesize,
        screenBuffer,
        this->outPixelFormat,
        pCodecCtxOfDecoder->width,
        pCodecCtxOfDecoder->height,
        1
    );
    // 设置pFrameYUV格式等参数
    pFrameYUV->setFrameParams(this->outPixelFormat, pCodecCtxOfDecoder->width, pCodecCtxOfDecoder->height);
    this->pFrameOfScreen = pFrameOfScreen;
    this->pFrameYUV = pFrameYUV;
    this->isRunning = true;
}

Scaler::~Scaler() {
    sws_freeContext(this->imgConvertCtx);
}

AVPixelFormat Scaler::getPixelFormat() const {
    return this->outPixelFormat;
}

void Scaler::stopScaling() {
    this->isRunning = false;
}

void Scaler::run() {
    while (isRunning) {
        // pFrameYUV的生产者, pFrameOfScreen的消费者
        pFrameYUV->empty.acquire();
        pFrameOfScreen->full.acquire();
        this->scale();
        pFrameYUV->full.release();
        pFrameOfScreen->empty.release();
    }
}

void Scaler::scale() {
    sws_scale(
        this->imgConvertCtx,
        pFrameOfScreen->getFrame()->data,
        pFrameOfScreen->getFrame()->linesize,
        0,
        pFrameYUV->getFrame()->height,
        pFrameYUV->getFrame()->data,
        pFrameYUV->getFrame()->linesize
    );
}
