#include "encoder.h"

Encoder::Encoder(FrameShared *frame, PacketShared *packet, const AVCodecID codecType, const AVPixelFormat pixelFormat, int inWidth, int inHeight) {
    /** ==============h264编码: YUV420P --> H264 设置相关参数 ================= */
    pCodecOfEncoder = avcodec_find_encoder(codecType);
    if (!pCodecOfEncoder) {
        qDebug() << "---------h264 codec not found--------";
        return;
    }
    // 设置encoder上下文的参数
    pCodecCtxOfEncoder = avcodec_alloc_context3(pCodecOfEncoder);
    pCodecCtxOfEncoder->codec_id = pCodecOfEncoder->id;
    pCodecCtxOfEncoder->codec_type = pCodecOfEncoder->type;
    pCodecCtxOfEncoder->pix_fmt = pixelFormat;
    pCodecCtxOfEncoder->width = inWidth;
    pCodecCtxOfEncoder->height = inHeight;
    // 编码质量速度相关参数
    pCodecCtxOfEncoder->time_base.num = 1;
    pCodecCtxOfEncoder->time_base.den = 60;    //帧率(既一秒钟多少张图片)
    pCodecCtxOfEncoder->bit_rate = 8e6;        //比特率(调节这个大小可以改变编码后视频的质量)
    pCodecCtxOfEncoder->gop_size = 12;
    if (pCodecCtxOfEncoder->flags & AVFMT_GLOBALHEADER)
        pCodecCtxOfEncoder->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    // 编码器相关参数
    AVDictionary *params = NULL;
    av_dict_set(&params, "preset", "superfast", 0);
    av_dict_set(&params, "tune", "zerolatency", 0);
    if (avcodec_open2(pCodecCtxOfEncoder, pCodecOfEncoder, &params) < 0) {
        qDebug() << "Failed to open video encoder! 编码器打开失败";
        return;
    }
    this->frame = frame;
    this->packet = packet;
    this->isRunning = true;
}

Encoder::~Encoder() {
    avcodec_free_context(&this->pCodecCtxOfEncoder);
}

void Encoder::stopEncoding() {
    this->isRunning = false;
}

void Encoder::run() {
    while (isRunning) {
        // packet的生产者, frame的消费者
        packet->empty.acquire();
        frame->full.acquire();
        this->encode();
        packet->full.release();
        frame->empty.release();
    }
}

void Encoder::encode() {
    int ret = avcodec_send_frame(pCodecCtxOfEncoder, frame->getFrame());
    if (ret < 0) {
        qDebug() << "Encoder: send frame failed.";
        return;
    }
    ret = avcodec_receive_packet(pCodecCtxOfEncoder, packet->getPacket());
    if (ret < 0) {
        qDebug() << "Encoder: receive packet failed.";
        return;
    }
}
