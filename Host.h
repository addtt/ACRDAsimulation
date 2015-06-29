//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __ACRDA_HOST_H_
#define __ACRDA_HOST_H_

#include <omnetpp.h>
#include "acrdaPkt.h"

namespace acrda {

/**
 * Aloha host; see NED file for more info.
 */
class Host : public cSimpleModule
{
  private:
    // parameters
    simtime_t radioDelay;
    //double txRate;
    cPar *iaTime;
    cPar *pkLenBits;

    static const int N_REP = 3;
    static const int N_SLOTS = 10;
    const double T_FRAME = 1;
    const double T_PKT_MAX = T_FRAME / N_SLOTS;   // Slot duration

    const double PKDURATION = T_PKT_MAX * 0.9;  // TODO: TEMPORARY!

    static const int MSG_STARTFRAME = 1;
    static const int MSG_STARTTX    = 2;
    static const int MSG_ENDTX      = 3;

    // state variables, event pointers etc
    cModule *server;
    cMessage *startFrameEvent;
    cMessage *startTxEvent[N_REP];
    cMessage *endTxEvent[N_REP];
    enum {IDLE=0, TRANSMIT=1} state;
    simsignal_t stateSignal;
    int pkCounter;
    int replicaCounter;

    AcrdaPkt *framePkts[N_REP]; // Array of pointers to packet objects TODO: what should I do?

  public:
    Host();
    virtual ~Host();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    simtime_t getNextTransmissionTime();
};

}; //namespace

#endif

