#include "AcrdaPkt.h"

AcrdaPkt::AcrdaPkt()
{
}

AcrdaPkt::AcrdaPkt(int hostIdx, int pkIdx, double pkGenerationTime, const char *msg,
        std::vector<double> replicaOffsets, double snr, int sf) : cPacket(msg)
{
    this->snr = snr;
    this->hostIdx = hostIdx;
    this->pkIdx = pkIdx;
    this->generationTime = pkGenerationTime;
    this->replicaOffsets = replicaOffsets;
    this->sf = sf;
}

AcrdaPkt::~AcrdaPkt()
{
}
