#include "acrdaPkt.h"

//Register_Class(acrdaPkt);


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
