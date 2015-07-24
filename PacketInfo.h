/*
 * PacketInfo.h
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */


#ifndef PACKETINFO_H_
#define PACKETINFO_H_

#include <acrdaPkt.h>

class PacketInfo {

public:
    PacketInfo() {};
    PacketInfo(AcrdaPkt *pkt, simtime_t startTime, simtime_t endTime);
    PacketInfo(int hostIdx, int pkIdx, double snr, std::vector<double> replicaOffsets, bool resolved, simtime_t startTime, simtime_t endTime);
    virtual ~PacketInfo();
    bool operator==(const PacketInfo &other) const;
    bool operator!=(const PacketInfo &other) const;

    int getHostIdx() const;
    int getPkIdx() const;
    bool isResolved() const {return resolved;}
    void setResolved(bool resolved=true);
    double getSnr() const;
    simtime_t getStartTime() const;
    simtime_t getEndTime() const;
    bool isReplicaOf(PacketInfo *p);


private:
    int hostIdx;
    int pkIdx;
    double snr;
    std::vector<double> replicaOffsets;
    bool resolved;
    simtime_t startTime;
    simtime_t endTime;

};

#endif /* PACKETINFO_H_ */
