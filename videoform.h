#ifndef VIDEOFORM_H
#define VIDEOFORM_H

#include <QWidget>

#include "server.h"
#include "decoder.h"
#include "frames.h"
#include "render.h"
#include "androidconvertnormal.h"

namespace Ui {
class VideoForm;
}

class VideoForm : public QWidget {
    Q_OBJECT

public:
    explicit VideoForm(const QString &serial, QWidget *parent = nullptr);
    ~VideoForm();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void wheelEvent(QWheelEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    void updateShowSize(const QSize &newSize);

    Ui::VideoForm *ui;
    QString serial = "";
    Server server;
    Decoder decoder;
    Frames frames;
    AndroidConvertNormal inputConverter;
    QSize frameSize;
};

#endif // VIDEOFORM_H
