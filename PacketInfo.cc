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
    sf = pkt->getSpreadingFactor();
    generationTime = pkt->getGenerationTime();
    resolved = false;
}

PacketInfo::PacketInfo(int hostIdx, int pkIdx, double snr, int sf,
        bool resolved, simtime_t generationTime, simtime_t startTime, simtime_t endTime)
    : hostIdx{hostIdx}, pkIdx{pkIdx}, snr{snr}, sf{sf}, resolved{resolved},
      generationTime{generationTime}, startTime{startTime}, endTime{endTime}
{}


PacketInfo::~PacketInfo() {
}

bool PacketInfo::operator==(const PacketInfo &other) const {
    return (hostIdx == other.hostIdx) &&
            (pkIdx == other.pkIdx) &&
            (snr == other.snr) &&
            (generationTime == other.generationTime) &&
            (resolved == other.resolved) &&
            (startTime == other.startTime) &&
            (endTime == other.endTime);
}

bool PacketInfo::operator!=(const PacketInfo &other) const {
    return !(*this == other);
}


void PacketInfo::setResolved(bool resolved) {
    this->resolved = resolved;
}


bool PacketInfo::isReplicaOf(PacketInfo *other)
{
    return (this->getHostIdx() == other->getHostIdx() && this->getPkIdx() == other->getPkIdx());
}

