#ifndef __ACRDAPKT_H__
#define __ACRDAPKT_H__

#include <omnetpp.h>

class AcrdaPkt : public cPacket
{
private:
    int hostIdx;
    int pkIdx;
    double snr;
    double *replicaOffsets;


public:
    AcrdaPkt(int hostIdx, int pkIdx, const char *msg, double replicaOffs[], double pktSnr=10);
    AcrdaPkt(const AcrdaPkt& pkt);
    ~AcrdaPkt();

    double getSnr() { return snr;}
    double *getReplicaOffs()   { return replicaOffsets;}
    int getHostIdx()  { return hostIdx;}
    int getPkIdx()  { return pkIdx;}
    virtual AcrdaPkt *dup() const { return new AcrdaPkt(*this);} // why const (overloads stuff...)? why virtual?
    void copy(const AcrdaPkt& pkt);
};

#endif
