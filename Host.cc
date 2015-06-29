//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "Host.h"
#include "acrdaPkt.h"

namespace acrda {

Define_Module(Host);


Host::Host()
{
    startFrameEvent = NULL;
    server = NULL;

    //TODO: init other stuff to NULL...?
}


Host::~Host()
{
    std::cout << "Destructor called - host\n";
    cancelAndDelete(startFrameEvent);
    for (int i=0; i<N_REP; i++) {
        if (startTxEvent[i] != NULL)
            cancelAndDelete(startTxEvent[i]);
        if (endTxEvent[i] != NULL)
            cancelAndDelete(endTxEvent[i]);
    }
}


void Host::initialize()
{
    // TODO all constants must be defined here. Then they will take values from the .ini parameters.

    stateSignal = registerSignal("state");
    server = simulation.getModuleByPath("server");
    if (!server) error("server not found");

    //txRate = par("txRate");
    radioDelay = par("radioDelay");
    iaTime = &par("iaTime");
    pkLenBits = &par("pkLenBits");

    replicaCounter = 0;

    //framePkts = new AcrdaPkt* [N_REP];
    //startTxEvent = new cMessage *[N_REP];
    //endTxEvent   = new cMessage *[N_REP];

    startFrameEvent = new cMessage("startFrameEvent", MSG_STARTFRAME); // always the same object
    for (int i=0; i<N_REP; i++) {
        startTxEvent[i] = new cMessage("startTxEvent", MSG_STARTTX);
        endTxEvent[i]   = new cMessage("endTxEvent",   MSG_ENDTX);
    }


    state = IDLE;
    emit(stateSignal, state);
    pkCounter = 0;
    WATCH((int&)state);
    WATCH(pkCounter);

    if (ev.isGUI())
        getDisplayString().setTagArg("t",2,"#808000");

    scheduleAt(0, startFrameEvent);
}


void Host::handleMessage(cMessage *msg)
{
    ASSERT(msg->getKind()==MSG_STARTFRAME || msg->getKind() == MSG_STARTTX || msg->getKind() == MSG_ENDTX);

    if (msg->getKind()==MSG_STARTFRAME)
    {
        ASSERT (state==IDLE);

        scheduleAt(simTime() + T_FRAME, startFrameEvent);

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

        // Display replica locations
        for (int i=0; i<N_REP; i++)
            EV << replicaLocs[i] << " ";
        EV << endl;

        // Generate packets and schedule start times
        char pkname[40];
        sprintf(pkname,"pk-%d-#%d", this->idx, pkCounter);
        for (int i=0; i<N_REP; i++) {   // for each packet replica in this frame
            double replicaFrameOffset = replicaLocs[i] * T_PKT_MAX;
            scheduleAt(simTime() + replicaFrameOffset, startTxEvent[i]);

            // Take all replica offsets except the current one, center them around the current one
            // TODO: We don't need this
            double replicaRelativeOffsets[N_REP-1];
            for (int j=0; j<N_REP-1; j++) {
                int shiftIdx = (j>=i) ? 1 : 0;
                replicaRelativeOffsets[j] = (replicaLocs[j + shiftIdx] - replicaLocs[i]) * T_PKT_MAX;
            }

            // Create the current packet
//            if (framePkts[i] != NULL)  // why does this result in malloc error?!
//                delete framePkts[i];
            framePkts[i] = new AcrdaPkt(this->idx, pkCounter, pkname, replicaRelativeOffsets);
            framePkts[i]->setBitLength(pkLenBits->longValue()); // TODO: useless?
        }

        pkCounter++;
        replicaCounter = 0;

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


        simtime_t duration = PKDURATION; // TODO handle duration as required!
        AcrdaPkt *outPkt = framePkts[replicaCounter]->dup();
        sendDirect(outPkt, radioDelay, duration, server->gate("in"));
        delete framePkts[replicaCounter];
        //sendDirect(framePkts[replicaCounter], radioDelay, duration, server->gate("in"));
//        cPacket *tmpmsg = new cPacket("packet");
//        sendDirect(tmpmsg, radioDelay, duration, server->gate("in"));
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

simtime_t Host::getNextTransmissionTime()
{
    return (simTime() + iaTime->doubleValue());
}

}; //namespace
