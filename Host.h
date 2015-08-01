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
#include "AcrdaPkt.h"
#include <fstream>
#include <random>

namespace acrda {

enum ArrivalType { EXTERNAL, HEAVY_TRAFFIC, POISSON };

class Host : public cSimpleModule
{
  private:

    // Parameters

    // Random number generator
    std::default_random_engine generator; // TODO use one per distribution?

    simtime_t radioDelay;   // Propagation delay from this host to the server
    int N_REP;          // Number of replicas in a frame, for each packet (including itself)
    int N_SLOTS;        // Number of slots in a frame
    double T_FRAME;     // Frame duration
    double T_PKT_MAX;   // Slot duration
    double PKDURATION;  // Transmission time of a packet
    ArrivalType arrivalType;

    double avgSnrLinear;    // Average SNR for this host, linear
    std::lognormal_distribution<double> lognormalDist;  // Get log-normal distributed values for the SNR

    // Parameters and variables for the Poisson case
    double meanInterarrival; // Mean interarrival time for the Poisson case

    bool haveDataFile;
    std::string arrivalsFileName;
    std::string dataFileName;
    std::ifstream arrivalsFile;
    std::ifstream dataFile;

    int thisHostsId;    // ID of the current host: it is the index of the host array (from 0 to numHosts-1)

    // Constants
    static const int MSG_STARTFRAME = 1;
    static const int MSG_STARTTX    = 2;
    static const int MSG_ENDTX      = 3;

    // State variables, event pointers etc
    cModule *server;
    cMessage *startFrameEvent;
    cMessage **startTxEvent;
    cMessage **endTxEvent;
    enum {IDLE=0, TRANSMIT=1} state;    // TODO Not needed
    simsignal_t stateSignal;            // Not needed
    int pkCounter;      // Packet counter for this host: it goes from 0 up for each _new_ packet
    int replicaCounter; // For each frame, this counts the number of replicas sent up to now.
    std::vector<AcrdaPkt> framePkts; // Array of packet objects
    std::vector<double> arrivalTimes;           // Vector of arrival times, if given in the data file
    std::vector<double>::iterator arrTimesIter; // Iterator for the vector of arrival times

  public:
    Host();
    virtual ~Host();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}; //namespace

#endif

