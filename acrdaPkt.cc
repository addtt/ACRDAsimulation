#include "acrdaPkt.h"

//Register_Class(acrdaPkt);

AcrdaPkt::AcrdaPkt(const AcrdaPkt& pkt) : cPacket(pkt)
{
    copy(pkt);
}

AcrdaPkt::AcrdaPkt(int hostIdx, int pkIdx, const char *msg, std::vector<double> replicaOffsets, double snr) : cPacket(msg)
{
    this->snr = snr;
    this->hostIdx = hostIdx;
    this->pkIdx = pkIdx;
    this->replicaOffsets = replicaOffsets;
}

AcrdaPkt::~AcrdaPkt()
{
}

void AcrdaPkt::copy(const AcrdaPkt& pkt)
{
    snr = pkt.getSnr();
    hostIdx = pkt.getHostIdx();
    pkIdx = pkt.getPkIdx();
    replicaOffsets = pkt.getReplicaOffs();
}

std::vector<double> AcrdaPkt::getReplicaOffs() const
{
    return replicaOffsets;
}
