/*
 * PacketInfo.cc
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#include <PacketInfo.h>

PacketInfo::PacketInfo(AcrdaPkt *pkt, simtime_t startTime, simtime_t endTime) {
    this->startTime = startTime;
    this->endTime = endTime;
    hostIdx = pkt->getHostIdx();
    pkIdx = pkt->getPkIdx();
    snr = pkt->getSnr();
    replicaOffsets = pkt->getReplicaOffs();
    resolved = false;
}

PacketInfo::~PacketInfo() {
}

simtime_t PacketInfo::getEndTime() const {
    return endTime;
}

int PacketInfo::getHostIdx() const {
    return hostIdx;
}

int PacketInfo::getPkIdx() const {
    return pkIdx;
}

void PacketInfo::setResolved(bool resolved) {
    this->resolved = resolved;
}

double PacketInfo::getSnr() const {
    return snr;
}

simtime_t PacketInfo::getStartTime() const {
    return startTime;
}

bool PacketInfo::isReplicaOf(PacketInfo *other)
{
    return (this->getHostIdx() == other->getHostIdx() && this->getPkIdx() == other->getPkIdx());
}
