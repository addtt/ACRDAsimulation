#include "AcrdaPkt.h"

AcrdaPkt::AcrdaPkt()
{
}

AcrdaPkt::AcrdaPkt(int hostIdx, int pkIdx, double pkGenerationTime, const char *msg,
        double snr, int sf) : cPacket(msg)
{
    this->snr = snr;
    this->hostIdx = hostIdx;
    this->pkIdx = pkIdx;
    this->generationTime = pkGenerationTime;
    this->sf = sf;
}

AcrdaPkt::~AcrdaPkt()
{
}
