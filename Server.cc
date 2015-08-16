//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "Server.h"

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
    maxSf = par("maxSpreadingFactor");
    wndLength = par("wndLength").doubleValue() * maxSf;
    wndShift = par("wndShift").doubleValue() * maxSf;
    numIterIC = par("numIterIC");
    double sinrThresh_dB = par("sinrThresh_dB");
    std::cout << "Server: SINR threshold is " << sinrThresh_dB << " dB" << endl;
    sinrThresh = pow(10, sinrThresh_dB / 10);   // SINR threshold in linear

    rxWnd = AcrdaWnd(wndLength);
    rxWnd.setSinrThresh(sinrThresh);

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

    scheduleAt(wndLength, wndCompleted);

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

        // We may have received replicas of old packets. Use the old packets to flag the new
        // replicas as resolved (the receiver will perform IC as in an usual iteration).
        // For this to work _always_, we need that for each host the frame length is smaller
        // than window size - window shift + 2*(transmission time for a packet).
        rxWnd.updateResolvedFlagsOfReplicas();

        // Perform IC iterations
        EV << "\nInterference Cancellation\n";
        for (int iter=0; iter < numIterIC; iter++) {

            // Get all packets that are directly resolvable, i.e. they do not collide with non-resolved
            // packets. These packets are used to perform IC using replicas.
            std::vector<int> newResolvableIds = rxWnd.getNewResolvableIndices();
            for (int i=0; i < newResolvableIds.size(); i++) {
                PacketInfo resolvablePkt = rxWnd.get(newResolvableIds.at(i));

                // Flag all replicas of that packet (including itself) as resolved
                for (int j=0; j < rxWnd.size(); j++) {
                    PacketInfo p = rxWnd.get(j);
                    if (p.isReplicaOf(& resolvablePkt))
                        rxWnd.setPacketResolvedFlag(j);
                }
            }

            // If no new resolvable packets exist, break the loop. This could be because all
            // pkts have been resolved, or because no more pkts can be resolved. The latter
            // case is called loop phenomenon [ACRDA_paper] and [CRDSA_paper].
            // TODO: consider correlation between loop phenomena in overlapping windows
            if (newResolvableIds.size() == 0) {
                icIterationsHist[iter]++;
                break;
            }

            //EV << "End of iteration " << (iter+1);
            //EV << rxWnd.toString();
            //EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;
        }

        EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;

        // This is the case of loop phenomenon. Related to the IC failure rate.
        if (! rxWnd.areAllResolved())
            loopEvents++;

        std::vector<PacketInfo> resolvedPkts = rxWnd.getResolvedPkts();
        updateResolvedPktsLists(successfulPackets, resolvedPkts);

        // Shift the window (discard oldest packets).
        double newWndLeft = simTime().dbl() + wndShift - wndLength;
        rxWnd.shift(newWndLeft);
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
    recordScalar("duration", simTime());

    EV << "\n\n\n";
    EV << "Simulation duration: " << simTime() << endl;

    std::ostringstream strStream;

    // Compute number of attempted packets for each host.
    // It is the ceiling of the number of received packets divided by the number of replicas.
    // In fact the server module receives all packets, even the colliding ones, and knows
    // everything about them. Therefore the server module receives a number of packets equal
    // to a multiple of the number of replicas N_REP, plus a number of replicas of the last
    // packet that is strictly smaller than N_REP.
    for (int i=0; i<numHosts; i++)
        numAttemptedPackets[i] = ceil((((double)numReceivedPackets[i]) / N_REP));

    // Compute number of successful packets for each host.
    for (int i=0; i<numHosts; i++)
        numSuccessfulPackets[i] = successfulPackets[i].size();

    // Compute success rate of the system and of each host
    std::vector<double> successRates(numHosts);
    double systemSuccessRate = 0;
    for (int i=0; i<numHosts; i++) {
        successRates[i] = (double)numSuccessfulPackets[i] / numAttemptedPackets[i];
        systemSuccessRate += successRates[i] / numHosts;
    }


    // Display number of received packets (including replicas) and successful ones, for each host.
    strStream << "\t\tRcvd (w/replicas)    Attempted       Successful\n";
    for (int i=0; i<numHosts; i++) {
        strStream << "From host " << i << ":\t\t";
        strStream << numReceivedPackets[i] << "\t\t" << numAttemptedPackets[i] << "\t\t" << numSuccessfulPackets[i];
        strStream << endl;
    }

    // Display success rate statistics
    strStream << "\n\nSuccess rate\n";
    for (int i=0; i<numHosts; i++)
        strStream << "  host " << i << ": " << successRates[i] << endl;
    strStream << "  total : " << systemSuccessRate << endl;


    // Compute throughput of the system and of each host (pkts per second)
    double sysThrput = 0;
    std::vector<double> hostThrput(numHosts);
    for (int i=0; i<numHosts; i++) {
        hostThrput[i] = ((double) numSuccessfulPackets[i]) / simTime();
        sysThrput += hostThrput[i];
    }

    // Display throughput statistics
    strStream << "\n\nThroughput (packets per second)\n";
    for (int i=0; i<numHosts; i++)
        strStream << "  host " << i << ": " << hostThrput[i] << endl;
    strStream << "  total : " << sysThrput << endl;



    // Display IC iterations statistics
    strStream << "\nIC iterations:\n";
    for (int i=0; i<numIterIC; i++)
        strStream << i << " iterations: " << icIterationsHist[i] << endl;

    // Display empirical loop probability
    strStream << "\nNumber of loop events: " << loopEvents << " (" << (((double)loopEvents) / wndShiftEvents * 100) << "% of wnd shifts)\n";

    strStream << "\n\n";
    std::string logString = strStream.str();


    // --- PRINT

    // Print to cout
    std::cout << "\n\n";
    std::cout << logString;
    std::cout.flush();

    // Retrieve time and define name of logfile
    time_t rawtime;
    struct tm * timeinfo;
    char timecstring [30];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timecstring,30,"%Y%m%d-%H%M%S",timeinfo);
    std::string timeStr = timecstring;
    std::string logFileName = "outputfiles/acrda-" + timeStr + ".log";

    // Open and print to log file
    std::ofstream logFile (logFileName, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        logFile << logString;
        logFile.close();
        std::cout << "Successfully written log to file\n";
    }
    else
        std::cerr << "Unable to open log file in write mode\n";

}




}; //namespace
