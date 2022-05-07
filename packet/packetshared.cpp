#include "packetshared.h"

PacketShared::PacketShared(): empty(1), full(0) {
    packetShared = av_packet_alloc();
}

PacketShared::~PacketShared() {
    av_packet_free(&packetShared);
    packetShared = Q_NULLPTR;
}

AVPacket *PacketShared::getPacket() {
    return packetShared;
}
