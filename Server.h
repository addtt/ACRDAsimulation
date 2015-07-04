//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#ifndef __ACRDA_SERVER_H_
#define __ACRDA_SERVER_H_

#include <omnetpp.h>
#include <acrdaPkt.h>
#include <PacketInfo.h>
#include <AcrdaWnd.h>

namespace acrda {

/**
 * Aloha server; see NED file for more info.
 */
class Server : public cSimpleModule
{
  private:
    int numHosts;       // Number of hosts
    double wndSize;    // Window size in seconds
    double wndShift;   // Window shift in seconds
    int numIterIC;       // Number of iterations for IC

    // state variables, event pointers
    bool nowReceiving;
    cMessage *endRxEvent;
    cMessage *wndCompleted;
    AcrdaWnd rxWnd;

    long numIncomingTransmissions;
    simtime_t recvStartTime;
    enum {IDLE=0, TRANSMISSION=1, COLLISION=2};

    // statistics
    simsignal_t receiveBeginSignal;
    simsignal_t receiveSignal;
    std::vector<long> decodedPackets;  // TODO: implement this part
    std::vector<int> numReceivedPackets;

  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  private:
    int updateResolvedPktsLists(std::vector< std::list<int> > &allDecodedPackets, std::vector<PacketInfo> const &resolvedPkt);
};

}; //namespace

#endif

