#include "AcrdaPkt.h"

AcrdaPkt::AcrdaPkt()
{
}

AcrdaPkt::AcrdaPkt(int hostIdx, int pkIdx, const char *msg, std::vector<double> replicaOffsets, double snr, int sf) : cPacket(msg)
{
    this->snr = snr;
    this->hostIdx = hostIdx;
    this->pkIdx = pkIdx;
    this->replicaOffsets = replicaOffsets;
    this->sf = sf;
}

AcrdaPkt::~AcrdaPkt()
{
}
