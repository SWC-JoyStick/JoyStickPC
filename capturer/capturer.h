#ifndef CAPTURER_H
#define CAPTURER_H

#include <QThread>
#include <QDebug>
#include "packetshared.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
}
// '1' Use Dshow
// '0' Use VFW
#define USE_DSHOW 0

class Capturer: public QThread {
    Q_OBJECT
public:
    Capturer(PacketShared *packet);
    virtual ~Capturer();
    AVFormatContext *getFormatCtx() const;
    int getVideoIndex() const;

public slots:
    void stopCapturing();

protected:
    void run();

private:
    PacketShared *packet;
    /** ========== 录屏 ScreenFormatCtx ===========*/
    AVFormatContext *pFormatCtxOfScreen;
    int videoIndex;
    bool isRunning;

    void capture();
};

#endif // CAPTURER_H
