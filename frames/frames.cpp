#include "frames.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

Frames::Frames() {

}

Frames::~Frames() {

}

bool Frames::init() {
    decodingFrame = av_frame_alloc();
    if (!decodingFrame)     // 初始化失败
        goto error;

    renderingFrame = av_frame_alloc();
    if (!renderingFrame)
        goto error;

    isRenderingFrameConsumed = true;
    return true;

error:
    destroy();
    return false;
}

void Frames::destroy() {
    if (decodingFrame) {
        av_frame_free(&decodingFrame);
        decodingFrame = Q_NULLPTR;
    }
    if (renderingFrame) {
        av_frame_free(&renderingFrame);
        renderingFrame = Q_NULLPTR;
    }
}

void Frames::lock() {
    mutex.lock();
}

void Frames::unlock() {
    mutex.unlock();
}

AVFrame *Frames::getDecodingFrame() {
    return decodingFrame;
}

bool Frames::offerDecodedFrame() {
    // 互斥访问decoding/rendering frame相关变量
    mutex.lock();
    /*
    if (renderExpiredFrames) {
        // 必须等当前帧渲染完成后再输入下一帧
        while (!isRenderingFrameConsumed && !interrupted)
            renderingFrameConsumedCond.wait(&mutex);
    }
    */
    swap();
    bool isPreviousFrameConsumed = isRenderingFrameConsumed;
    isRenderingFrameConsumed = false;
    mutex.unlock();
    return isPreviousFrameConsumed;
}

const AVFrame *Frames::consumeRenderedFrame() {
    // 使用掉当前渲染好的帧
    Q_ASSERT(!isRenderingFrameConsumed);
    isRenderingFrameConsumed = true;
    /*
    if (renderExpiredFrames)
        // 如果要渲染之前的帧, 则可以唤醒decoder, 让其可以push帧到renderer中了
        renderingFrameConsumedCond.wakeOne();
    */
    return renderingFrame;
}

void Frames::stop() {

}

void Frames::swap() {
    AVFrame *tmp = decodingFrame;
    decodingFrame = renderingFrame;
    renderingFrame = tmp;
}
