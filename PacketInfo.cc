/*
 * PacketInfo.cc
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#include <PacketInfo.h>

PacketInfo::PacketInfo(AcrdaPkt *pkt, simtime_t startTime, simtime_t endTime)
    : startTime{startTime}, endTime{endTime}
{
    hostIdx = pkt->getHostIdx();
    pkIdx = pkt->getPkIdx();
    snr = pkt->getSnr();
    replicaOffsets = pkt->getReplicaOffs();
    resolved = false;
}

PacketInfo::PacketInfo(int hostIdx, int pkIdx, double snr, std::vector<double> replicaOffsets,
        bool resolved, simtime_t startTime, simtime_t endTime)
    : hostIdx{hostIdx}, pkIdx{pkIdx}, snr{snr}, replicaOffsets{replicaOffsets},
      resolved{resolved}, startTime{startTime}, endTime{endTime}
{}


PacketInfo::~PacketInfo() {
}

bool PacketInfo::operator==(const PacketInfo &other) const {
    return (hostIdx == other.hostIdx) &&
            (pkIdx == other.pkIdx) &&
            (snr == other.snr) &&
            (replicaOffsets == other.replicaOffsets) &&
            (resolved == other.resolved) &&
            (startTime == other.startTime) &&
            (endTime == other.endTime);
}

bool PacketInfo::operator!=(const PacketInfo &other) const {
    return !(*this == other);
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

