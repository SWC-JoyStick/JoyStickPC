#ifndef PACKETSHARED_H
#define PACKETSHARED_H

#include <QCoreApplication>
#include <QSemaphore>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

class PacketShared {
public:
    QSemaphore empty;
    QSemaphore full;

    PacketShared();
    ~PacketShared();
    AVPacket *getPacket();

private:
    AVPacket *packetShared;
};

#endif // PACKETSHARED_H
