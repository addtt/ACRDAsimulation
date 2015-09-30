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
#include <PacketInfo.h>
#include <AcrdaWnd.h>
#include <fstream>
#include <time.h>

namespace acrda {

/**
 * Aloha server; see NED file for more info.
 */
class Server : public cSimpleModule
{
  private:

    // Network parameters
    int numHosts;       // Number of hosts
    double wndLength;   // Window size in seconds
    double wndShift;    // Window shift in seconds
    int numIterIC;      // Number of iterations for IC
    int N_REP;          // Total number of replicas in one frame
    double sinrThresh;  // Linear threshold for SINR
    int maxSf;          // Max SF for all the system. The receive wnd size is proportional to maxSf.

    // state variables, event pointers
    bool nowReceiving;              // True if and only if the server is receiving
    long numIncomingTransmissions;  // Number of incoming transmission at this moment
    simtime_t recvStartTime; // Start of continuous reception from one or more hosts, if nowReceiving is true.
    AcrdaWnd rxWnd;    // Receive window
    cMessage *endRxEvent;
    cMessage *wndCompleted;

    enum {IDLE=0, TRANSMISSION=1, COLLISION=2};

    // statistics
    simsignal_t receiveBeginSignal;
    simsignal_t receiveSignal;
    std::vector< std::list<int> > successfulPackets;
    std::vector<int> numReceivedPackets;
    std::vector<int> numSuccessfulPackets;
    std::vector<int> numAttemptedPackets;
    std::vector<int> icIterationsHist;  // The number of iterations is the index of the vector plus 1.
    int loopEvents;     // Number of loop phenomena
    int wndShiftEvents; // Number of window shifts
    std::vector<double> avgGlobalDelays;    // It contains the cumulative sum of delays until the avg is computed in the end.

  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  private:
    int updateResolvedPktsLists(std::vector< std::list<int> > &allDecodedPackets,
            std::vector<double> &avgGlobalDelays, std::vector<PacketInfo> const &resolvedPkt);
};

}; //namespace

#endif

