#include "videoform.h"
#include "ui_videoform.h"

#define HEIGHT 950
#define WIDTH 420

VideoForm::VideoForm(const QString &serial, QWidget *parent): QWidget(parent), ui(new Ui::VideoForm), serial(serial) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(&server, &Server::serverStartResult, this, [this](bool success) {
        qDebug() << "server start: " << success;
    });
    connect(&server, &Server::connectionResult, this, [this](bool success, const QString &deviceName, const QSize &size) {
        qDebug() << "connection result: " << success << deviceName << size;
        if (success) {
            this->setWindowTitle(deviceName);
            updateShowSize(size);
            decoder.setDeviceSocket(server.getDeviceSocket());
            decoder.startDecode();
            inputConverter.setControlSocket(server.getDeviceSocket());
        }
    });

    frames.init();
    decoder.setFrames(&frames);

    connect(&server, &Server::onServerStop, this, [this](){
        close();
#ifdef QT_DEBUG
        qDebug() << "server process stop.";
#endif
    });

    connect(&decoder, &Decoder::onDecodeStop, this, [this]() {
        close();
#ifdef QT_DEBUG
        qDebug() << "decoder thread stop.";
#endif
    });

    connect(&decoder, &Decoder::onNewFrame, this, [this]() {
#ifdef QT_DEBUG
        qDebug() << "Signal: Decoder::onNewFrame.";
#endif
        frames.lock();
        // 读取frame, 传入OpenGL进行渲染
        const AVFrame *frame = frames.consumeRenderedFrame();
        // 更新窗口大小
        updateShowSize(QSize(frame->width, frame->height));
        ui->videoWidget->setFrameSize(QSize(frame->width, frame->height));
        ui->videoWidget->updateTextures(frame->data[0], frame->data[1], frame->data[2], frame->linesize[0], frame->linesize[1], frame->linesize[2]);
        frames.unlock();
    });
    updateShowSize(this->size());
    server.start(serial, 27183, 1080, 8e6);
}

VideoForm::~VideoForm() {
    server.stop();
    decoder.stopDecode();
    frames.destroy();
    delete ui;
}

void VideoForm::mousePressEvent(QMouseEvent *event) {
    inputConverter.mouseEvent(event, ui->videoWidget->getFrameSize(), ui->videoWidget->size());
}

void VideoForm::mouseReleaseEvent(QMouseEvent *event) {
    inputConverter.mouseEvent(event, ui->videoWidget->getFrameSize(), ui->videoWidget->size());
}

void VideoForm::mouseMoveEvent(QMouseEvent *event) {
    inputConverter.mouseEvent(event, ui->videoWidget->getFrameSize(), ui->videoWidget->size());
}

void VideoForm::wheelEvent(QWheelEvent *event) {
    inputConverter.wheelEvent(event, ui->videoWidget->getFrameSize(), ui->videoWidget->size());
}

void VideoForm::keyPressEvent(QKeyEvent *event) {
    inputConverter.keyEvent(event, ui->videoWidget->getFrameSize(), ui->videoWidget->size());
}

void VideoForm::keyReleaseEvent(QKeyEvent *event) {
    inputConverter.keyEvent(event, ui->videoWidget->getFrameSize(), ui->videoWidget->size());
}

void VideoForm::updateShowSize(const QSize &newSize) {
    if (frameSize != newSize) {
        frameSize = newSize;
        bool vertical = newSize.height() > newSize.width();
        if (vertical)
            resize(WIDTH, HEIGHT);
        else
            resize(HEIGHT, WIDTH);
    }
}
