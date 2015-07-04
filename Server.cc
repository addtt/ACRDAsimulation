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

    numReceivedPackets.resize(numHosts);
    decodedPackets.resize(numHosts);

    gate("in")->setDeliverOnReceptionStart(true);
    nowReceiving = false;
    numIncomingTransmissions = 0;

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

            EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;
        }

        // Shift the window
        double newWndLeft = simTime().dbl() + wndShift - wndSize;
        rxWnd.shift(newWndLeft);  // Shift (and update 'resolved' flags)

        // Schedule next window shift
        scheduleAt(simTime() + wndShift, wndCompleted);
    }


    // --- End of reception: just for statistics
    // The channel seen by the server becomes idle.

    else if (msg==endRxEvent)
    {
        nowReceiving = false;
        numIncomingTransmissions = 0;
        emit(receiveBeginSignal, numIncomingTransmissions);

        // update network graphics
        if (ev.isGUI()) {
            getDisplayString().setTagArg("i2",0,"x_yellow");
            getDisplayString().setTagArg("t",0, ("Receiving" + std::to_string(numIncomingTransmissions)).c_str() );
            getDisplayString().setTagArg("t",2,"#808000");
        }
    }


    // --- Received a packet from a host (start of reception)

    else
    {
        AcrdaPkt *pkt = check_and_cast<AcrdaPkt *>(msg);
        emit(receiveBeginSignal, ++numIncomingTransmissions);
        ASSERT(pkt->isReceptionStart());  // this packet object represents the start of the reception (at the server)
        simtime_t recvStartTime = simTime();
        simtime_t recvEndTime = recvStartTime + pkt->getDuration(); // end-of-reception time (at the server)

        numReceivedPackets[pkt->getHostIdx()]++;

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
            getDisplayString().setTagArg("t",0, ("Receiving" + std::to_string(numIncomingTransmissions)).c_str() );
            getDisplayString().setTagArg("t",2,"#808000");
        }

        nowReceiving = true;
        delete pkt;
    }
}


int Server::updateResolvedPktsLists(std::vector< std::list<int> > &allDecodedPackets, std::vector<PacketInfo> const &resolvedPkt)
{
    std::vector<PacketInfo>::const_iterator iter_wnd; // Iterator for the vector of resolved packets in this window.
    std::list<int>::reverse_iterator iter_list;  // Iterator for the list of all resolved pkts of one host: from most recent to oldest.

    int numNewResolvedPkts = 0;

    for (iter_wnd = resolvedPkt.begin(); iter_wnd != resolvedPkt.end(); iter_wnd++) {
        PacketInfo pkt = *iter_wnd;         // dereferencing: get one resolved packet
        int hostIdx = pkt.getHostIdx(); // host index of this packet
        int pkIdx = pkt.getPkIdx();     // packet index of this packet

        // Check the list of resolved packets for the specific host, see if this packet
        // was already resolved (either this packet or one of its replicas). Start from
        // the end of the list, since the packet can only be toward the end of the list.
        bool found = false;
        iter_list = allDecodedPackets[hostIdx].rbegin();
        while (true) { // TODO write this better
            if (iter_list == allDecodedPackets[hostIdx].rend() || *iter_list < pkIdx) {
                // It's not there: either we started to get old packets or we went through the whole list already.
                // Insert it and break loop if we're sure we will not find it.
                allDecodedPackets[hostIdx].insert(iter_list.base(), pkIdx); // Add it to the list of resolved packet (i.e. correctly received)
                numNewResolvedPkts++;
                break;
            }
            else if (*iter_list == pkIdx) {       // Break loop if we find it
                found = true;
                break;
            }
            iter_list++;
        }
    }
    return numNewResolvedPkts;
}



void Server::finish()
{
    EV << "duration: " << simTime() << endl;

    recordScalar("duration", simTime());
}



}; //namespace
