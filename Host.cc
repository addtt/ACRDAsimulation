//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "Host.h"

namespace acrda {

Define_Module(Host);


Host::Host()
{
    startFrameEvent = NULL;
    server = NULL;
}


Host::~Host()
{
    std::cout << "Destructor called - host" << thisHostsId << endl;
    std::cout.flush();
    cancelAndDelete(startFrameEvent);
    for (int i=0; i<N_REP; i++) {
        if (startTxEvent[i] != NULL)
            cancelAndDelete(startTxEvent[i]);
        if (endTxEvent[i] != NULL)
            cancelAndDelete(endTxEvent[i]);
    }
    delete [] startTxEvent;
    delete [] endTxEvent;
}


void Host::initialize()
{
    thisHostsId = this->idx;

    std::string arrivalTypeStr = par("arrivalType");
    if (arrivalTypeStr == "EXTERNAL")
        arrivalType = EXTERNAL;
    else if (arrivalTypeStr == "HEAVY_TRAFFIC")
        arrivalType = HEAVY_TRAFFIC;
    else if (arrivalTypeStr == "POISSON")
        arrivalType = POISSON;
    else
        throw cRuntimeError("Unknown 'arrival type' parameter");

    avgSnrLinear = par("defaultAvgSnrLinear");  // TODO: possibility to set default for each host in the data file
    spreadingFactor = par("defaultSpreadingFactor");  // TODO: possibility to set default for each host in the data file

    std::ostringstream dataStrStream;
    dataStrStream << "inputfiles/host" << thisHostsId << "_data.txt";   // TODO: this should be a parameter (.ini file)
    dataFileName = dataStrStream.str();
    dataFile = std::ifstream(dataFileName);

    if (dataFile.is_open()) {
        haveDataFile = true;
        std::string line;
        getline(dataFile, line);
        radioDelay = std::stod(line, nullptr);
        dataFile.close();
    }
    else {
        std::cout << "Unable to open file " << dataFileName << endl;
        haveDataFile = false;
    }

    if (arrivalType == EXTERNAL) {
        bool haveExternalArrivalTimes;

        std::ostringstream arrStrStream;
        arrStrStream << "inputfiles/host" << thisHostsId << "_arrivals.txt"; // TODO: this should be a parameter (.ini file)
        arrivalsFileName = arrStrStream.str();
        arrivalsFile = std::ifstream(arrivalsFileName);

        if (arrivalsFile.is_open()) {
            std::string line;
            while (getline(arrivalsFile, line))
                arrivalTimes.push_back(std::stod(line, nullptr));
            arrivalsFile.close();
            arrivalTimes.shrink_to_fit();
            arrTimesIter = arrivalTimes.begin();
            haveExternalArrivalTimes = (arrivalTimes.size() > 0);
        }
        else {
            std::cout << "Unable to open file " << arrivalsFileName << endl;
            haveExternalArrivalTimes = false;
        }
        if (!haveExternalArrivalTimes)
            throw cRuntimeError("Could not retrieve arrival times from the arrivals file");
    }

    else if (arrivalType == POISSON) {
        meanInterarrival = par("meanInterarr");
    }

    // Display input file status
    std::cout << "   hostId=" << thisHostsId;
    std::cout << "   haveDataFile=" << haveDataFile;
    std::cout << "   arrivalType=" << arrivalType;
    std::cout << endl;

    stateSignal = registerSignal("state");
    server = simulation.getModuleByPath("server");
    if (!server) error("server not found");

    if (!haveDataFile)
        radioDelay = par("radioDelay");
    N_REP = par("nRep");
    N_SLOTS = par("nSlots");
    T_FRAME = par("tFrame").doubleValue() * spreadingFactor;
    T_PKT_MAX = T_FRAME / N_SLOTS;
    PKDURATION = 0.9999 * T_PKT_MAX;  // Guard interval to avoid disaster

    replicaCounter = 0;
    pkCounter = 0;
    backlogEvents = 0;

    framePkts.resize(N_REP);
    startTxEvent = new cMessage *[N_REP];
    endTxEvent   = new cMessage *[N_REP];

    startFrameEvent = new cMessage("startFrameEvent", MSG_STARTFRAME); // always the same object
    for (int i=0; i<N_REP; i++) {
        startTxEvent[i] = new cMessage("startTxEvent", MSG_STARTTX);
        endTxEvent[i]   = new cMessage("endTxEvent",   MSG_ENDTX);
    }


    state = IDLE;
    emit(stateSignal, state);
    WATCH((int&)state);
    WATCH(pkCounter);

    if (ev.isGUI())
        getDisplayString().setTagArg("t",2,"#808000");

    simtime_t firstArrival;
    if (arrivalType == HEAVY_TRAFFIC) {
        firstArrival = 0;
        arrivalTimes.push_back(firstArrival.dbl());
    }
    else if (arrivalType == EXTERNAL) {
        firstArrival = *arrTimesIter;
        arrTimesIter++;
    }
    else if (arrivalType == POISSON) {
        firstArrival = (&par("randExpUnity"))->doubleValue() * meanInterarrival;
        arrivalTimes.push_back(firstArrival.dbl());
    }
    scheduleAt(firstArrival, startFrameEvent);
}


void Host::handleMessage(cMessage *msg)
{
    ASSERT(msg->getKind()==MSG_STARTFRAME || msg->getKind() == MSG_STARTTX || msg->getKind() == MSG_ENDTX);

    if (msg->getKind()==MSG_STARTFRAME)
    {
        ASSERT (state==IDLE);

        simtime_t nextStartFrame = simTime() + T_FRAME;
        if (arrivalType == EXTERNAL) {
            double nextArrival = *arrTimesIter; // It can actually be in the past if we're backlogged
            nextStartFrame = std::max(nextStartFrame.dbl(), nextArrival);
            if (nextArrival < simTime().dbl())
                backlogEvents++;
            arrTimesIter++;
        }
        else if (arrivalType == POISSON) {
            // Generate each time an interarrival and save each arrival event in a vector
            double lastArrival = arrivalTimes[arrivalTimes.size() - 1];
            double nextInterarr = (&par("randExpUnity"))->doubleValue() * meanInterarrival;
            double nextArrival = lastArrival + nextInterarr; // It can actually be in the past if we're backlogged
            arrivalTimes.push_back(nextArrival);
            nextStartFrame = std::max(nextStartFrame.dbl(), nextArrival);

            if (nextArrival < simTime().dbl())
                backlogEvents++;
            //    std::cout << simTime().dbl() << ":   \t" << lastArrival << " \t" << nextInterarr << " \t" << nextArrival << "  (" << nextStartFrame << ")" << endl;
        }
        else if (arrivalType == HEAVY_TRAFFIC) {
            arrivalTimes.push_back(nextStartFrame.dbl());
        }
        scheduleAt(nextStartFrame, startFrameEvent);

        // Choose replica locations
        int replicaLocs[N_REP];
        bool decidedReplicas = false;
        while (!decidedReplicas) {
            for (int i=0; i<N_REP; i++)
                replicaLocs[i] = floor((double)rand() / ((unsigned long)RAND_MAX + 1) * N_SLOTS);

            bool allUnique = true;
            for (int i=0; i<N_REP && allUnique; i++)
                for (int j=0; j<N_REP && allUnique; j++)
                    if (i!=j && replicaLocs[i] == replicaLocs[j])
                        allUnique = false;

            if (allUnique)
                decidedReplicas = true;
        }

        // Generate packets and schedule start times
        char pkname[40];
        sprintf(pkname,"pk-%d-#%d", thisHostsId, pkCounter);
        for (int i=0; i<N_REP; i++) {   // for each packet replica in this frame
            double replicaFrameOffset = replicaLocs[i] * T_PKT_MAX;
            scheduleAt(simTime() + replicaFrameOffset, startTxEvent[i]);

            // Take all replica offsets except the current one, center them around the current one
            // TODO: We don't need this
            std::vector<double> replicaRelativeOffsets(N_REP-1);
            for (int j=0; j<N_REP-1; j++) {
                int shiftIdx = (j>=i) ? 1 : 0;
                replicaRelativeOffsets[j] = (replicaLocs[j + shiftIdx] - replicaLocs[i]) * T_PKT_MAX;
            }

            // Create the current packet
            double snr = (&par("randSnrDistrib"))->doubleValue() * avgSnrLinear;
            avgSnr += snr;
            double pkArrivalTime = arrivalTimes[arrivalTimes.size()-1];
            framePkts[i] = AcrdaPkt(thisHostsId, pkCounter, pkArrivalTime, pkname, replicaRelativeOffsets, snr, spreadingFactor);
        }

        pkCounter++;        // Increment packet counter
        replicaCounter = 0; // Initialize replica counter for the current packet

    } // start-frame event


    else if (msg->getKind() == MSG_STARTTX)
    {
        ASSERT (state==IDLE);
        ASSERT (replicaCounter >= 0 && replicaCounter < N_REP);
        state = TRANSMIT;
        emit(stateSignal, state);

        // update network graphics
        if (ev.isGUI()) {
            getDisplayString().setTagArg("i",1,"yellow");
            getDisplayString().setTagArg("t",0,"TRANSMIT");
        }

        simtime_t duration = PKDURATION;
        sendDirect(framePkts[replicaCounter].dup(), radioDelay, duration, server->gate("in"));
        scheduleAt(simTime()+duration, endTxEvent[replicaCounter]);

        replicaCounter++; // number of replicas sent in this frame so far
    }

    else if (msg->getKind() == MSG_ENDTX)  // TODO We don't need this, actually we don't need the state.
    {
        ASSERT (state==TRANSMIT);
        state = IDLE;
        emit(stateSignal, state);

        // update network graphics
        if (ev.isGUI()) {
            getDisplayString().setTagArg("i",1,"");
            getDisplayString().setTagArg("t",0,"");
        }
    }

}

void Host::finish()
{
    //std::cout << "Host " << thisHostsId << ":  E[SNR] = " << (avgSnr/pkCounter/N_REP) << endl;
}

}; //namespace
