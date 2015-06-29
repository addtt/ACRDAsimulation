#include "acrdaPkt.h"

//Register_Class(acrdaPkt);

AcrdaPkt::AcrdaPkt(const AcrdaPkt& pkt) : cPacket(pkt)
{
    copy(pkt);
}

AcrdaPkt::AcrdaPkt(int hostIdx, int pkIdx, const char *msg, double replicaOffs[], double pktSnr) : cPacket(msg)
{
    snr = pktSnr;
    replicaOffsets = replicaOffs;
    this->hostIdx = hostIdx;
    this->pkIdx = pkIdx;
}

AcrdaPkt::~AcrdaPkt()
{
}

void AcrdaPkt::copy(const AcrdaPkt& pkt)
{
    snr = pkt.snr;
    replicaOffsets = pkt.replicaOffsets;
    hostIdx = pkt.hostIdx;
    pkIdx = pkt.pkIdx;
}
