#include "screendecoder.h"

ScreenDecoder::ScreenDecoder(PacketShared *packet, FrameShared *pFrameOfScreen, int videoIndex, AVFormatContext *pFormatCtxOfScreen): pCodecCtxOfDecoder(NULL) {
    /** ==============解码过程 创建解码器上下文, 用于解码录屏帧================= */
    // 获取录屏流Codec解码器上下文
    pCodecCtxOfDecoder = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(pCodecCtxOfDecoder, pFormatCtxOfScreen->streams[videoIndex]->codecpar);
    // 寻找并打开解码器
    pCodecOfDecoder = avcodec_find_decoder(pCodecCtxOfDecoder->codec_id);
    if (pCodecOfDecoder == NULL) {
        qDebug() << "Codec not found.";
        return;
    }
    if (avcodec_open2(pCodecCtxOfDecoder, pCodecOfDecoder, NULL)) {
        qDebug() << "Could not open codec.";
        return;
    }
    qDebug() << "width = " << pCodecCtxOfDecoder->width << ", height = " << pCodecCtxOfDecoder->height;   // 打印出视频的宽高
    // 设置解码用帧的参数
    qDebug() << pCodecCtxOfDecoder->pix_fmt;
    pFrameOfScreen->setFrameParams(pCodecCtxOfDecoder->pix_fmt, pCodecCtxOfDecoder->width, pCodecCtxOfDecoder->height);
    this->packet = packet;
    this->frame = pFrameOfScreen;
    this->isRunning = true;
}

ScreenDecoder::~ScreenDecoder() {
    avcodec_free_context(&this->pCodecCtxOfDecoder);
}

AVCodecContext *ScreenDecoder::getCodecCtx() const {
    return pCodecCtxOfDecoder;
}

void ScreenDecoder::stopDecoding() {
    this->isRunning = false;
}

void ScreenDecoder::run() {
    while (isRunning) {
        // frame的生产者, packet的消费者
        frame->empty.acquire();
        packet->full.acquire();
        this->decode();
        frame->full.release();
        packet->empty.release();
    }
}

void ScreenDecoder::decode() {
    int ret = avcodec_send_packet(pCodecCtxOfDecoder, packet->getPacket());
    if (ret < 0) {
        qDebug() << "Decoder: send packet failed.";
        return;
    }
    ret = avcodec_receive_frame(pCodecCtxOfDecoder, frame->getFrame());
    if (ret < 0) {
        qDebug() << "Decoder: receive frame failed.";
        return;
    }
}
