#include "acrdaPkt.h"

//Register_Class(acrdaPkt);


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

std::vector<double> AcrdaPkt::getReplicaOffs() const
{
    return replicaOffsets;
}
