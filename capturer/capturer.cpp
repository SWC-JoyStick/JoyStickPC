#include "capturer.h"

Capturer::Capturer(PacketShared *packet) {
    this->packet = packet;
    this->isRunning = true;
    this->pFormatCtxOfScreen = avformat_alloc_context();         // 录屏流上下文
    avdevice_register_all();        // 注册录屏设备
    // 抓取屏幕
#ifdef Q_OS_WIN32
#if USE_DSHOW
    // Use dshow
    //
    // Need to Install screen-capture-recorder
    // screen-capture-recorder
    // Website: http://sourceforge.net/projects/screencapturer/
    //
    AVInputFormat *ifmt = av_find_input_format("dshow");
    if(avformat_open_input(&pFormatCtxOfScreen, "video=screen-capture-recorder", ifmt, NULL) != 0) {
        qDebug() << "Couldn't open input stream.";
        return;
    }
#else
    // Use gdigrab
    const AVInputFormat* ifmt = av_find_input_format("gdigrab");
    if (avformat_open_input(&pFormatCtxOfScreen, "desktop", ifmt, NULL) != 0) {
        qDebug() << "Couldn't open input stream.";
        return;
    }
#endif
#elif defined Q_OS_LINUX
    // Linux
    const AVInputFormat *ifmt = av_find_input_format("x11grab");
    // Grab at position 10,20
    if (avformat_open_input(&pFormatCtxOfScreen, ":0.0+10,20", ifmt, NULL) != 0) {
        qDebug() << "Couldn't open input stream.";
        return;
    }
#else
    // Mac
    /** ==============打开设备，选出录屏视频流================= */
    const AVInputFormat *ifmt = av_find_input_format("avfoundation");
    // Avfoundation
    // [video]:[audio]
    if (avformat_open_input(&pFormatCtxOfScreen, "1", ifmt, NULL) != 0) {
        qDebug() << "Couldn't open input stream.";
        return;
    }
#endif
    if (avformat_find_stream_info(pFormatCtxOfScreen, NULL) < 0) {
        qDebug() << "Couldn't find stream information.";
        return;
    }
    //---------- 选取视频流 -------------------
    this->videoIndex = -1;
    for (int i = 0; i < pFormatCtxOfScreen->nb_streams; i++)
        if (pFormatCtxOfScreen->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            this->videoIndex = i;

    if (this->videoIndex == -1) {
        qDebug() << "Couldn't find a video stream.";
        return;
    }
}

Capturer::~Capturer() {
    avformat_free_context(this->pFormatCtxOfScreen);
}

AVFormatContext *Capturer::getFormatCtx() const {
    return this->pFormatCtxOfScreen;
}

int Capturer::getVideoIndex() const {
    return this->videoIndex;
}

void Capturer::stopCapturing() {
    this->isRunning = false;
}

void Capturer::run() {
    while (isRunning) {
        // packet的生产者
        packet->empty.acquire();
        this->capture();
        packet->full.release();
    }
}

void Capturer::capture() {
    int error = 0;
    while ((error = av_read_frame(pFormatCtxOfScreen, packet->getPacket())) < 0);
        // qDebug() << "Capturer: Error code = " << error;
}
