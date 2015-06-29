//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "Server.h"
#include <string>

namespace acrda {

Define_Module(Server);


Server::Server()
{
    endRxEvent = NULL;
}

Server::~Server()
{
    std::cout << "Destructor called - server\n";
    cancelAndDelete(endRxEvent);
    cancelAndDelete(wndCompleted);
    rxWnd.clear();
}

void Server::initialize()
{
    numHosts = par("numHosts");

    wndIterator = AcrdaWnd::Iterator(rxWnd);

    endRxEvent = new cMessage("end-reception");
    nowReceiving = false;

    gate("in")->setDeliverOnReceptionStart(true);

    receiveCounter = 0;

    receiveBeginSignal = registerSignal("receiveBegin");
    receiveSignal = registerSignal("receive");

    emit(receiveSignal, 0L);
    emit(receiveBeginSignal, 0L);

    wndCompleted = new cMessage("window-completed");
    scheduleAt(WND_SIZE, wndCompleted);

    if (ev.isGUI())
        getDisplayString().setTagArg("i2",0,"x_off");
}


void Server::handleMessage(cMessage *msg)
{

    if (msg==wndCompleted)
    {
        // Just display current window
        EV << "\n-------\nCurrent window:\n";
        wndIterator.init();
        while(!wndIterator.end()) {
            PacketInfo p = *((PacketInfo *) wndIterator++); // What if we used PacketInfo *p = (PacketInfo *) wndIterator++
            EV << "hostID=" << p.getHostIdx() << "\t\t";
            EV << p.getStartTime() << " to " << p.getEndTime() << "\n";
        }
        EV << "-------\n";

        // Perform IC iterations
        EV << "Interference Cancellation\n";
        for (int i=0; i < NUM_ITER; i++) {

            // Get the first resolvable (and not yet resolved) packet.
            PacketInfo *firstResPkt = (PacketInfo *) rxWnd.firstResolvable(); // TODO: How do we solve this???

            // Flag all replicas of the current packet (including itself) as resolved
            wndIterator.init();
            while(!wndIterator.end() && wndIterator.currElement() ) { //TODO: can this be simplified?
                PacketInfo *p = (PacketInfo *) wndIterator++;
                if (p->isReplicaOf(firstResPkt))
                    p->setResolved();
            }

            //numResolvedProgressive[i] = rxWnd.getNumResolved();
            //EV << "   resolved packets: " << rxWnd.getNumResolved() << endl;
        }

        // Shift the window
        double newWndLeft = simTime().dbl() + WND_SHIFT - WND_SIZE;
        rxWnd.shift(newWndLeft);  // Shift, defragment the internal array and update 'resolved' flags
        scheduleAt(simTime() + WND_SHIFT, wndCompleted); // TODO: should we delete msg before scheduling again?
    }


    else if (msg==endRxEvent)
    {
        nowReceiving = false;
        receiveCounter = 0;
        emit(receiveBeginSignal, receiveCounter);

        // update network graphics
        if (ev.isGUI()) {
            //getDisplayString().setTagArg("i2",0,"x_off");
            //getDisplayString().setTagArg("t",0,"");
            getDisplayString().setTagArg("i2",0,"x_yellow");
            getDisplayString().setTagArg("t",0, ("Receiving" + std::to_string(receiveCounter)).c_str() );
            getDisplayString().setTagArg("t",2,"#808000");
        }
    }


    // ------ Received a packet from a host (start of reception)

    else
    {
        AcrdaPkt *pkt = check_and_cast<AcrdaPkt *>(msg);
        emit(receiveBeginSignal, ++receiveCounter);
        ASSERT(pkt->isReceptionStart());  // this packet object represents the start of the reception (at the server)
        simtime_t recvEndTime = simTime() + pkt->getDuration(); // end-of-reception time (at the server)

        PacketInfo pkInfoObj = PacketInfo(pkt, simTime(), recvEndTime);
        rxWnd.add((cObject *) &pkInfoObj); // TODO: is this right?
//        PacketInfo *pkInfoObj = new PacketInfo(pkt, simTime(), recvEndTime);
//        rxWnd.add((cObject *) pkInfoObj);

        if (!nowReceiving) {
            // Set the channel as busy, schedule endRxEvent.
            recvStartTime = simTime();
            nowReceiving = true;
            scheduleAt(recvEndTime, endRxEvent);
        }
        else {
            if (recvEndTime > endRxEvent->getArrivalTime()) {
                cancelEvent(endRxEvent);
                scheduleAt(recvEndTime, endRxEvent);
            }
        }

        if (ev.isGUI()) {
            getDisplayString().setTagArg("i2",0,"x_yellow");
            getDisplayString().setTagArg("t",0, ("Receiving" + std::to_string(receiveCounter)).c_str() );
            getDisplayString().setTagArg("t",2,"#808000");
        }

        nowReceiving = true;
        delete pkt;
    }
}

void Server::finish()
{
    EV << "duration: " << simTime() << endl;

    recordScalar("duration", simTime());
}



}; //namespace
