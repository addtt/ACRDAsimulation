//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


#include "Server.h"
#include "Host.h"

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
    nSlots = par("nSlots");
    tPkt = par("tPkt");
    N_REP = par("nRep");
    maxSf = par("maxSpreadingFactor");
    tFrameServer = maxSf * tPkt * nSlots;
    wndLength = par("wndLengthNorm").doubleValue() * tFrameServer;
    wndShift = par("wndShiftNorm").doubleValue() * tFrameServer;
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
    avgGlobalDelays.resize(numHosts);

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

            //EV << "End of iteration " << (iter+1);
            //EV << rxWnd.toString();
            //EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;

            // If no new resolvable packets exist, break the loop. This could be because all
            // pkts have been resolved, or because no more pkts can be resolved. The latter
            // case is called loop phenomenon [ACRDA_paper] and [CRDSA_paper].
            // TODO: consider correlation between loop phenomena in overlapping windows
            if (rxWnd.getNewResolvableIndices().size() == 0) {
                icIterationsHist[iter]++;
                break;
            }
        }

        EV << "   resolved packets: " << rxWnd.getNumResolvedPkts() << endl;

        // This is the case of loop phenomenon. Related to the IC failure rate.
        if (! rxWnd.areAllResolved()) {
            loopEvents++;
            ev << "Loop event\n";
        }

        std::vector<PacketInfo> resolvedPkts = rxWnd.getResolvedPkts();
        updateResolvedPktsLists(successfulPackets, avgGlobalDelays, resolvedPkts);

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


int Server::updateResolvedPktsLists(std::vector< std::list<int> > &allDecodedPackets,
        std::vector<double> &avgGlobalDelays, std::vector<PacketInfo> const &resolvedPkt)
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

                // First we add it to the list of resolved packets (i.e. correctly received)
                allDecodedPackets[hostIdx].insert(iter_list.base(), pkIdx);
                // and then we add the packet's global delay to the cumulative sum of delays for that host.
                avgGlobalDelays[hostIdx] += (simTime() - pkt.getGenerationTime()).dbl();

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

    std::ostringstream consoleStream;
    std::ostringstream logStream;


    // --- Write system parameters to log file.

    // Get network module's handle, and some network parameters
    cModule *acrdaNetworkModule;
    acrdaNetworkModule = simulation.getModuleByPath("Acrda");
    if (!acrdaNetworkModule) error("Acrda network module not found");


    logStream << "SimulationTime," << simTime().dbl() << endl;
    logStream << "NumHosts,"       << numHosts << endl;
    logStream << "NumReplicas,"    << N_REP << endl;
    logStream << "NumSlots,"       << nSlots << endl;
    logStream << "FrameDurationAtServer,"  << tFrameServer << endl;
    logStream << "MaxICiterations,"<< numIterIC << endl;
    logStream << "MaxSF,"          << maxSf << endl;
    logStream << "WindowLength,"   << wndLength << endl;
    logStream << "WindowShift,"    << wndShift << endl;
    logStream << "SINRthreshold,"  << sinrThresh << endl;

    logStream << "\nHostNumber,SF,delay,arrivalType,mean interarrival,SNR distribution,avg SNR" << endl;
    for (int i=0; i<numHosts; i++) {
        std::string hostString = "host[" + std::to_string(i) + "]";
        cModule *currHost = simulation.getModuleByPath(hostString.c_str());
        if (!currHost) error("Host module not found");
        logStream << i << "," << ((Host *)currHost)->spreadingFactor << "," << ((Host *)currHost)->radioDelay
                << "," << acrdaNetworkModule->par("arrivalType").str() << "," << ((Host *)currHost)->meanInterarrival
                << "," << currHost->par("randSnrDistribStr").str() << "," << ((Host *)currHost)->avgSnrLinear << endl;
    }



    // --- Write statistics to log file (and to console)

    // Compute total number of received packets (with replicas)
    int numReceivedPacketsTot = 0;
    for (int i=0; i<numHosts; i++)
        numReceivedPacketsTot += numReceivedPackets[i];

    // Compute number of attempted packets for each host and in total.
    // It is the ceiling of the number of received packets divided by the number of replicas.
    // In fact the server module receives all packets, even the colliding ones, and knows
    // everything about them. Therefore the server module receives a number of packets equal
    // to a multiple of the number of replicas N_REP, plus a number of replicas of the last
    // packet that is strictly smaller than N_REP.
    int numAttemptedPacketsTot = 0;
    for (int i=0; i<numHosts; i++) {
        numAttemptedPackets[i] = ceil((((double)numReceivedPackets[i]) / N_REP));
        numAttemptedPacketsTot += numAttemptedPackets[i];
    }

    // Compute number of successful packets for each host and in total.
    int numSuccessfulPacketsTot = 0;
    for (int i=0; i<numHosts; i++) {
        numSuccessfulPackets[i] = successfulPackets[i].size();
        numSuccessfulPacketsTot += numSuccessfulPackets[i];
    }

    // Compute success rate of the system and of each host
    std::vector<double> successRates(numHosts);
    double systemSuccessRate = 0;
    for (int i=0; i<numHosts; i++) {
        successRates[i] = (double)numSuccessfulPackets[i] / numAttemptedPackets[i];
        systemSuccessRate += successRates[i] / numHosts;
    }

    // Compute throughput of the system and of each host (pkts per second)
    double sysThrput = 0;
    std::vector<double> hostThrput(numHosts);
    for (int i=0; i<numHosts; i++) {
        hostThrput[i] = ((double) numSuccessfulPackets[i]) / simTime();
        sysThrput += hostThrput[i];
    }


    // Display number of received packets (including replicas), successful ones, success rate and throughput, for each host.
    consoleStream << "\t\tRcvd (w/replicas)    Attempted       Successful     Succ rate      Throughput\n";
    for (int i=0; i<numHosts; i++) {
        consoleStream << "From host " << i << ":\t\t";
        consoleStream << numReceivedPackets[i] << "\t\t" << numAttemptedPackets[i] << "\t\t" << numSuccessfulPackets[i]
                  << "\t\t" << successRates[i] << "\t\t" << hostThrput[i];
        consoleStream << endl;
    }



    // Save number of received packets (including replicas), successful ones, success rate and throughput, for each host.
    logStream << "\nHostNumber,Rcvd (w/replicas),Attempted,Successful,Success Rate,Throughput\n";
    for (int i=0; i<numHosts; i++) {
        logStream << i << "," << numReceivedPackets[i] << "," << numAttemptedPackets[i]
                << "," << numSuccessfulPackets[i] << "," << successRates[i] << "," << hostThrput[i] << endl;
    }
    logStream << "all," << numReceivedPacketsTot << "," << numAttemptedPacketsTot << ","
            << numSuccessfulPacketsTot << "," << systemSuccessRate << "," << sysThrput << endl;


    // Compute average global delays by dividing the cumulative sum by the number of resolved packets for each host
    for (int i=0; i < numHosts; i++)
        avgGlobalDelays[i] /= numSuccessfulPackets[i];

    // Display delay statistics
    consoleStream << "\n\nGlobal delay (average)\n";
    for (int i=0; i<numHosts; i++)
        consoleStream << "  host " << i << ": " << avgGlobalDelays[i] << endl;

    // Save delay statistics
    logStream << "\nHostNumber,Global delay (average)\n";
    for (int i=0; i<numHosts; i++)
        logStream << i << "," << avgGlobalDelays[i] << endl;



    // Display IC iterations statistics
    consoleStream << "\nIC iterations:\n";
    for (int i=0; i<numIterIC; i++)
        consoleStream << (i+1) << " iterations: " << icIterationsHist[i] << endl;
    logStream << "\nIC iterations,Count\n";
    for (int i=0; i<numIterIC; i++)
        logStream << (i+1) << "," << icIterationsHist[i] << endl;

    // Display empirical loop probability
    consoleStream << "\nNumber of loop events: " << loopEvents << " (" << (((double)loopEvents) / wndShiftEvents * 100) << "% of wnd shifts)\n";
    logStream << "\nNumber of loop events," << loopEvents << endl << "Number of wnd shifts," << wndShiftEvents << endl;

    consoleStream << "\n\n";
    std::string consoleString = consoleStream.str();
    std::string logString = logStream.str();


    // --- PRINT

    // Print to cout
    std::cout << "\n\n";
    std::cout << consoleString;
    std::cout.flush();

    // Define name of logfile
    //time_t rawtime;
    //struct tm * timeinfo;
    //char timecstring [30];
    //time(&rawtime);
    //timeinfo = localtime(&rawtime);
    //strftime(timecstring,30,"%Y%m%d-%H%M%S",timeinfo);
    //std::string nameStr = timecstring;
    std::string nameStr = simulation.getActiveEnvir()->getConfigEx()->getVariable("runid");
    std::replace( nameStr.begin(), nameStr.end(), ':', '-'); // replace all ':' to '-'
    std::string logFileName = "outputfiles/acrda-" + nameStr + ".log";

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
