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
//#include <iostream>
#include <fstream>

namespace acrda {

/**
 * Aloha host; see NED file for more info.
 */
class Host : public cSimpleModule
{
  private:

    // Parameters

    simtime_t radioDelay;
    //double txRate;
    cPar *iaTime;
    cPar *pkLenBits;
    int N_REP;
    int N_SLOTS;
    double T_FRAME;
    double T_PKT_MAX;   // Slot duration
    double PKDURATION;
    std::vector<double> arrivalTimes;
    std::vector<double>::iterator arrTimesIter;

    bool haveDataFile;
    bool haveExternalArrivalTimes;
    std::string filename;
    std::ifstream dataFile;

    int thisHostsId;

    // Constants
    static const int MSG_STARTFRAME = 1;
    static const int MSG_STARTTX    = 2;
    static const int MSG_ENDTX      = 3;

    // State variables, event pointers etc
    cModule *server;
    cMessage *startFrameEvent;
    cMessage **startTxEvent;
    cMessage **endTxEvent;
    enum {IDLE=0, TRANSMIT=1} state;
    simsignal_t stateSignal;
    int pkCounter;
    int replicaCounter;
    AcrdaPkt **framePkts; // Array of pointers to packet objects

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

