#include "decoder.h"

#define BUFSIZE 0x10000

// 数据读取回调函数, 解码时会回调read_packet来获取视频流
static qint32 read_packet(void *opaque, quint8 *buf, qint32 bufSize) {
    Decoder *decoder = (Decoder *)opaque;
    if (decoder)
        return decoder->recvData(buf, bufSize);
    return 0;
}

Decoder::Decoder() {

}

Decoder::~Decoder() {

}

bool Decoder::init() {
    if (avformat_network_init())
        return false;
    return true;
}

void Decoder::destroy() {
    avformat_network_deinit();
}

void Decoder::setFrames(Frames *frames) {
    this->frames = frames;
}

void Decoder::setDeviceSocket(DeviceSocket *deviceSocket) {
    this->deviceSocket = deviceSocket;
}

qint32 Decoder::recvData(quint8 *buf, qint32 bufSize) {
    if (!buf)
        return 0;
    if (deviceSocket) {
        qint32 len = deviceSocket->subThreadRecvData(buf, bufSize);
#ifdef QT_DEBUG
        qDebug() << "Length of receiving data: " << len;
#endif
        if (len == -1)
            return AVERROR(errno);
        if (len == 0)
            return AVERROR_EOF;
        return len;
    }
    return AVERROR_EOF;
}

bool Decoder::startDecode() {
    if (!this->deviceSocket)
        return false;
    isQuit = false;
    start();        // 开始解码
    return true;
}

void Decoder::stopDecode() {
    isQuit = false;
    if (this->frames)
        this->frames->stop();       // 释放解码帧
    wait();         // 等待解码线程退出
}

void Decoder::run() {
    unsigned char *decoderBuffer = Q_NULLPTR;       // 保存解码数据的临时缓冲区
    AVIOContext *avioCtx = Q_NULLPTR;               // io上下文, 用于读取数据
    AVFormatContext *formatCtx = Q_NULLPTR;         // 封装上下文, 用于解封装
    AVCodecContext *codecCtx = Q_NULLPTR;           // 解码器上下文, 配合解码器
    const AVCodec *codec = Q_NULLPTR;               // 解码器
    bool isFormatCtxOpen = false;                   // formatCtx是否打开
    bool isCodecCtxOpen = false;                    // codecCtx是否打开

    // 1. 申请解码缓冲区
    decoderBuffer = (unsigned char *)av_malloc(BUFSIZE);
    if (!decoderBuffer) {
        qCritical("Could not allocate buffer.");
        goto runQuit;
    }

    // 2. 初始化I/O上下文
    avioCtx = avio_alloc_context(decoderBuffer, BUFSIZE, 0, this, read_packet, NULL, NULL);
    if (!avioCtx) {
        qCritical("Could not allocate avio context.");
        goto runQuit;
    }

    // 3. 初始化封装上下文
    formatCtx = avformat_alloc_context();
    if (!formatCtx) {
        qCritical("Could not allocate format context.");
        goto runQuit;
    }
    // 为封装上下文指定io上下文
    formatCtx->pb = avioCtx;
    // 打开封装上下文
    if (avformat_open_input(&formatCtx, NULL, NULL, NULL) < 0) {
        qCritical("Could not open video stream.");
        goto runQuit;
    }
    isFormatCtxOpen = true;

    // 4. 初始化解码器
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        qCritical("decoder not found.");
        goto runQuit;
    }

    // 5. 设置解码器上下文
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        qCritical("Could not allocate decoder context");
        goto runQuit;
    }
    // 打开解码器上下文
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        qCritical("Could not open codec");
        goto runQuit;
    }
    isCodecCtxOpen = true;

    // 6. 解码
    AVPacket packet;            // 视频解码数据包: 保存解码前的一帧视频流数据
    av_init_packet(&packet);    // 初始化packet
    packet.data = Q_NULLPTR;
    packet.size = 0;

    // 从封装上下文读取一阵解码前的数据, 保存到AVPacket中
    while (!isQuit && !av_read_frame(formatCtx, &packet)) {
        AVFrame *decodingFrame = frames->getDecodingFrame();           // 读取AVFrame用于保存解码的YUV数据帧
        // 解码过程, 将解码前的packet发送到解码器上下文解码
        int ret;
        if ((ret = avcodec_send_packet(codecCtx, &packet)) < 0) {
            qCritical("Could not send video packet: %d.", ret);
            goto runQuit;
        }
        if (decodingFrame)          // 将解码后的YUV帧存储到decodingFrame中
            ret = avcodec_receive_frame(codecCtx, decodingFrame);   // 成功返回0
        if (!ret) {
#ifdef QT_DEBUG
            qDebug() << "Pushing frame.";
#endif
            pushFrame();           // 将解码后的YUV帧推送出去, 用于显示
        } else if (ret != AVERROR(EAGAIN)) {      // AVERROR(EAGAIN): 解码器没有填满, avcodec_send_packet()输入不足, 不输出解码的帧
            qCritical("Could not receive video frame: %d.", ret);
            goto runQuit;
        }

        av_packet_unref(&packet);

        if (avioCtx->eof_reached)
            break;
    }
#ifdef QT_DEBUG
    qDebug() << "End of Frames.";
#endif

runQuit:
    // 解码过程结束/异常退出的善后处理
    if (avioCtx)
        av_freep(&avioCtx);
    if (formatCtx && isFormatCtxOpen)
        avformat_close_input(&formatCtx);
    if (formatCtx)
        avformat_free_context(formatCtx);
    if (codecCtx && isCodecCtxOpen)
        avcodec_close(codecCtx);
    if (codecCtx)
        avcodec_free_context(&codecCtx);

    emit onDecodeStop();
}

void Decoder::pushFrame() {
    bool isPreviousFrameConsumed = frames->offerDecodedFrame();
    if (!isPreviousFrameConsumed)
        return;
    emit onNewFrame();
}
