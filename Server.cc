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
    N_REP = par("nRep");
    wndSize = par("wndSize");
    wndShift = par("wndShift");
    numIterIC = par("numIterIC");

    numReceivedPackets.resize(numHosts);
    numSuccessfulPackets.resize(numHosts);
    numAttemptedPackets.resize(numHosts);
    successfulPackets.resize(numHosts);
    icIterationsHist.resize(numIterIC);
    loopEvents = 0;
    wndShiftEvents = 0;

    endRxEvent = new cMessage("end-reception");
    wndCompleted = new cMessage("window-completed");

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
        for (int iter=0; iter < numIterIC; iter++) {

            // Get the first resolvable (and not yet resolved) packet.
            PacketInfo firstResPkt;
            try {
                firstResPkt = rxWnd.firstResolvable();
            }
            catch (const std::out_of_range& oor) {
                // If no other resolvable packets exist, break the loop. This could be because all
                // pkts have been resolved, or because no more pkts can be resolved. The latter
                // case is called loop phenomenon [ACRDA_paper].
                icIterationsHist[iter]++;
                break;
            }

            // Flag all replicas of that packet (including itself) as resolved
            for (int j=0; j<rxWnd.size(); j++) {
                PacketInfo p = rxWnd.get(j);
                if (p.isReplicaOf(& firstResPkt)) {
                    p.setResolved();
                    rxWnd.addAt(j, p);
                }
            }

            //EV << rxWnd.toString();
            //EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;
        }

        // Now we update resolved flags according to replicas. If one packet is resolved, we flag all
        // of its replicas in the window as resolved. We could not do this before, because we needed
        // to distinguish between already resolved packets and those that are not yet resolved but
        // can be because they don't overlap with non-resolved packets.
        rxWnd.updateResolvedFlagsOfReplicas();

        EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;

        // This is the case of loop phenomenon. Related to the IC failure rate.
        // TODO Check definition of loop phenomenon! How should it behave with packets whose replicas fall in the future?
        if (! rxWnd.areAllResolved())
            loopEvents++;

        std::vector<PacketInfo> resolvedPkts = rxWnd.getResolvedPkts();
        updateResolvedPktsLists(successfulPackets, resolvedPkts);

        // Shift the window
        double newWndLeft = simTime().dbl() + wndShift - wndSize;
        rxWnd.shift(newWndLeft);  // Shift (and update 'resolved' flags after shifting)
        wndShiftEvents++;

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
    EV << "\n\n\n";
    EV << "Simulation duration: " << simTime() << endl;
    std::cout << "\n\n";

    recordScalar("duration", simTime());

    // Compute number of successful packets for each host.
    for (int i=0; i<numHosts; i++)
        numSuccessfulPackets[i] = successfulPackets[i].size();



    // Compute number of attempted packets for each host.
    // It is the ceiling of the number of received packets divided by the number of replicas.
    // In fact the server module receives all packets, even the colliding ones, and knows
    // everything about them. Therefore the server module receives a number of packets equal
    // to a multiple of the number of replicas N_REP, plus a number of replicas of the last
    // packet that is strictly smaller than N_REP.
    for (int i=0; i<numHosts; i++)
        numAttemptedPackets[i] = ceil((((double)numReceivedPackets[i]) / N_REP));

    // Compute throughput of the system and of each host (pkts per second)
    double sysThrput = 0;
    std::vector<double> hostThrput(numHosts);
    for (int i=0; i<numHosts; i++) {
        hostThrput[i] = ((double) numSuccessfulPackets[i]) / simTime();
        sysThrput += hostThrput[i];
    }

    // Display number of received packets (including replicas) and successful ones, for each host.
    std::cout << "\t\tRcvd (w/replicas)    Attempted       Successful\n";
    for (int i=0; i<numHosts; i++) {
        std::cout << "From host " << i << ":\t\t";
        std::cout << numReceivedPackets[i] << "\t\t" << numAttemptedPackets[i] << "\t\t" << numSuccessfulPackets[i];
        std::cout << endl;
    }

    // Display throughput statistics
    std::cout << "\n\nThroughput (packets per second)\n";
    for (int i=0; i<numHosts; i++)
        std::cout << "  host " << i << ": " << hostThrput[i] << endl;
    std::cout << "  total : " << sysThrput << endl;

    std::cout << "\nIC iterations:\n";
    for (int i=0; i<numIterIC; i++)
        std::cout << i << " iterations: " << icIterationsHist[i] << endl;

    std::cout << "\nNumber of loop events: " << loopEvents << " (" << (((double)loopEvents) / wndShiftEvents) << "% of wnd shifts)\n";

    std::cout << "\n\n";
    std::cout.flush();
}



}; //namespace
