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
    int numHosts;
    const double WND_SIZE = 3;    // Window size in seconds
    const double WND_SHIFT = 1; // Window shift in seconds
    static const int NUM_ITER = 5;      // Number of iterations for IC
    AcrdaWnd::Iterator wndIterator;  // TODO check this: it calls the default constructor in AcrdaWnd::Iterator (?

    // state variables, event pointers
    bool nowReceiving;
    cMessage *endRxEvent;
    cMessage *wndCompleted;
    AcrdaWnd rxWnd;

    long receiveCounter;
    simtime_t recvStartTime;
    enum {IDLE=0, TRANSMISSION=1, COLLISION=2};

    //int numResolvedProgressive[NUM_ITER];

    // statistics
    simsignal_t receiveBeginSignal;
    simsignal_t receiveSignal;

  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

}; //namespace

#endif

