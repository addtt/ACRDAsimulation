#ifndef __ACRDAPKT_H__
#define __ACRDAPKT_H__

#include <omnetpp.h>

class AcrdaPkt : public cPacket
{
private:
    int hostIdx;
    int pkIdx;
    double snr;     // Linear SNR
    int sf;         // Spreading Factor
    std::vector<double> replicaOffsets; // TODO Useless member


public:
    AcrdaPkt();
    AcrdaPkt(int hostIdx, int pkIdx, const char *msg, std::vector<double> replicaOffsets, double snr, int sf);
    ~AcrdaPkt();

    double getSnr() const { return snr;}
    std::vector<double> getReplicaOffs() const { return replicaOffsets; }
    int getHostIdx() const { return hostIdx;}
    int getPkIdx() const { return pkIdx;}
    int getSpreadingFactor() const { return sf; }

    virtual AcrdaPkt *dup() const { return new AcrdaPkt(*this);}
};

#endif
