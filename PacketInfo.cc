/*
 * PacketInfo.cc
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#include <PacketInfo.h>

PacketInfo::PacketInfo(AcrdaPkt *pkt, simtime_t startTime, simtime_t endTime)
    : startTime(startTime), endTime(endTime)
{
    hostIdx = pkt->getHostIdx();
    pkIdx = pkt->getPkIdx();
    snr = pkt->getSnr();
    replicaOffsets = pkt->getReplicaOffs();
    resolved = false;
}

PacketInfo::PacketInfo(int hostIdx, int pkIdx, double snr, std::vector<double> replicaOffsets, bool resolved, simtime_t startTime, simtime_t endTime) {
    this->startTime = startTime;
    this->endTime = endTime;
    this->hostIdx = hostIdx;
    this->pkIdx = pkIdx;
    this->snr = snr;
    this->replicaOffsets = replicaOffsets;
    this->resolved = resolved;
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

PacketInfo *PacketInfo::dup()
{
    return new PacketInfo(hostIdx, pkIdx, snr, replicaOffsets, resolved, startTime, endTime);
}
