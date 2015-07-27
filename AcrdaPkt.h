#ifndef __ACRDAPKT_H__
#define __ACRDAPKT_H__

#include <omnetpp.h>

class AcrdaPkt : public cPacket
{
private:
    int hostIdx;
    int pkIdx;
    double snr;
    std::vector<double> replicaOffsets;


public:
    AcrdaPkt();
    AcrdaPkt(int hostIdx, int pkIdx, const char *msg, std::vector<double> replicaOffsets, double snr=10);
    ~AcrdaPkt();

    double getSnr() const { return snr;}
    std::vector<double> getReplicaOffs() const;
    int getHostIdx() const { return hostIdx;}
    int getPkIdx() const { return pkIdx;}
    virtual AcrdaPkt *dup() const { return new AcrdaPkt(*this);}
};

#endif
