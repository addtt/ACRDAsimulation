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
    endRxEvent = nullptr;
    wndCompleted = nullptr;
}

Server::~Server()
{
    std::cout << "Destructor called - server\n";
    std::cout.flush();
    cancelAndDelete(endRxEvent);
    cancelAndDelete(wndCompleted);
    rxWnd.clear();
}

void Server::initialize()
{
    numHosts = par("numHosts");
    wndSize = par("wndSize");
    wndShift = par("wndShift");
    numIterIC = par("numIterIC");

    endRxEvent = new cMessage("end-reception");
    wndCompleted = new cMessage("window-completed");

    gate("in")->setDeliverOnReceptionStart(true);
    nowReceiving = false;
    receiveCounter = 0;

    receiveBeginSignal = registerSignal("receiveBegin");
    receiveSignal = registerSignal("receive");

    emit(receiveSignal, 0L);
    emit(receiveBeginSignal, 0L);

    scheduleAt(wndSize, wndCompleted);

    if (ev.isGUI())
        getDisplayString().setTagArg("i2",0,"x_off");
}


void Server::handleMessage(cMessage *msg)
{

    // --- Completed window

    if (msg==wndCompleted)
    {
        // Display current window
        EV << rxWnd.toString();

        // Perform IC iterations
        EV << "Interference Cancellation\n";
        for (int i=0; i < numIterIC; i++) {

            // Get the first resolvable (and not yet resolved) packet.
            PacketInfo firstResPkt;
            try {
                firstResPkt = rxWnd.firstResolvable();
            }
            catch (const std::out_of_range& oor) { // TODO check this
                break;
                //std::cerr << "Out of Range error: " << oor.what() << '\n';
                //std::cerr << simTime() << endl;
                //exit(1);
            }

            // Flag all replicas of that packet (including itself) as resolved
            for (int j=0; j<rxWnd.size(); j++) {
                PacketInfo p = rxWnd.get(j);
                if (p.isReplicaOf(& firstResPkt)) {
                    p.setResolved();
                    rxWnd.addAt(j, p);
                }
            }

            EV << "   resolved packets: " << rxWnd.getNumResolved() << endl;
        }

        // Shift the window
        double newWndLeft = simTime().dbl() + wndShift - wndSize;
        rxWnd.shift(newWndLeft);  // Shift (and update 'resolved' flags)

        // Schedule next window shift
        scheduleAt(simTime() + wndShift, wndCompleted);
    }


    // --- End of reception: just for statistics

    else if (msg==endRxEvent)
    {
        nowReceiving = false;
        receiveCounter = 0;
        emit(receiveBeginSignal, receiveCounter);

        // update network graphics
        if (ev.isGUI()) {
            getDisplayString().setTagArg("i2",0,"x_yellow");
            getDisplayString().setTagArg("t",0, ("Receiving" + std::to_string(receiveCounter)).c_str() );
            getDisplayString().setTagArg("t",2,"#808000");
        }
    }


    // --- Received a packet from a host (start of reception)

    else
    {
        AcrdaPkt *pkt = check_and_cast<AcrdaPkt *>(msg);
        emit(receiveBeginSignal, ++receiveCounter);
        ASSERT(pkt->isReceptionStart());  // this packet object represents the start of the reception (at the server)
        simtime_t recvEndTime = simTime() + pkt->getDuration(); // end-of-reception time (at the server)

        PacketInfo pkInfoObj = PacketInfo(pkt, simTime(), recvEndTime);
        rxWnd.add(pkInfoObj);

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
