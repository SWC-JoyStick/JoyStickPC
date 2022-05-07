/**
 * 本程序实现了屏幕录制功能。可以录制并播放桌面数据。是基于FFmpeg
 * 的libavdevice类库最简单的例子。通过该例子，可以学习FFmpeg中
 * libavdevice类库的使用方法。
 * 本程序在Windows下可以使用2种方式录制屏幕：
 *  1.gdigrab: Win32下的基于GDI的屏幕录制设备。
 *             抓取桌面的时候，输入URL为“desktop”。
 *  2.dshow: 使用Directshow。注意需要安装额外的软件screen-capture-recorder
 * 在Linux下可以使用x11grab录制屏幕。
 * 在MacOS下可以使用avfoundation录制屏幕。
 *
 * This software capture screen of computer. It's the simplest example
 * about usage of FFmpeg's libavdevice Library.
 * It's suiltable for the beginner of FFmpeg.
 * This software support 2 methods to capture screen in Microsoft Windows:
 *  1.gdigrab: Win32 GDI-based screen capture device.
 *             Input URL in avformat_open_input() is "desktop".
 *  2.dshow: Use Directshow. Need to install screen-capture-recorder.
 * It use x11grab to capture screen in Linux.
 * It use avfoundation to capture screen in MacOS.
 */

#include "screentoandroid.h"

ScreenToAndroid::ScreenToAndroid(QObject *parent): QObject(parent) {
    // 创建解码所用frame
    capturer = new Capturer(&packetOfScreen);
    decoder = new ScreenDecoder(&packetOfScreen, &pFrameOfScreen, capturer->getVideoIndex(), capturer->getFormatCtx());
    scaler = new Scaler(&pFrameOfScreen, &pFrameYUV, decoder->getCodecCtx(), AV_PIX_FMT_YUV420P);
    encoder = new Encoder(&pFrameYUV, &packetOfSocket, AV_CODEC_ID_H264, scaler->getPixelFormat(), decoder->getCodecCtx()->width, decoder->getCodecCtx()->height);
    sender = new Sender(&packetOfSocket);
    sender->startListening();

    connect(sender, &Sender::onConnected, this, [this]() {
        capturer->start();
        decoder->start();
        scaler->start();
        encoder->start();
        sender->start();
        converter->setControlSocket(sender->getDeviceSocket());
    });
    connect(sender, &Sender::onDisconnected, this, [this]() {
        this->close();
        /**
        qDebug() << "emit: onClosed()";
        emit onClosed();
        */
    });
}

void ScreenToAndroid::close() {
    capturer->stopCapturing();
    decoder->stopDecoding();
    scaler->stopScaling();
    encoder->stopEncoding();
    sender->stopSending();
    sender->stop();
    qDebug() << "All modules closed.";
    // 等待子线程退出
}
