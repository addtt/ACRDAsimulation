#ifndef __ACRDAPKT_H__
#define __ACRDAPKT_H__

#include <omnetpp.h>

class AcrdaPkt : public cPacket
{
private:
    int hostIdx;
    int pkIdx;
    double snr;     // Linear SNR
    double generationTime;  // Time of arrival at the sender's buffer, i.e. time of generation of the packet at the sender.
    int sf;         // Spreading Factor


public:
    AcrdaPkt();
    AcrdaPkt(int hostIdx, int pkIdx, double pkGenerationTime, const char *msg,
            double snr, int sf);
    ~AcrdaPkt();

    double getSnr() const { return snr;}
    int getHostIdx() const { return hostIdx;}
    int getPkIdx() const { return pkIdx;}
    int getSpreadingFactor() const { return sf; }
    double getGenerationTime() const { return generationTime; }

    virtual AcrdaPkt *dup() const { return new AcrdaPkt(*this);}
};

#endif
