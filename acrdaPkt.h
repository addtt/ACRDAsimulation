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
    ~AcrdaPkt();

    double getSnr() { return snr;}
    double *getReplicaOffs()   { return replicaOffsets;}
    int getHostIdx()  { return hostIdx;}
    int getPkIdx()  { return pkIdx;}
};

#endif
