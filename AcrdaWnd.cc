/*
 * AcrdaWnd.cc
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#include <AcrdaWnd.h>
#include <sstream>
#include <vector>


AcrdaWnd::~AcrdaWnd()
{
    clear();
}


void AcrdaWnd::clear()
{
    vect.clear();
}


void AcrdaWnd::add(PacketInfo obj)
{
    if (obj.getStartTime() < wndLeft)
        std::cerr << "Warning! Trying to add old packet in the receive window" << endl;
    else
        vect.push_back(obj);
}

void AcrdaWnd::addAt(int m, PacketInfo obj)
{
    if (obj.getStartTime() < wndLeft)
        std::cerr << "Warning! Trying to add old packet in the receive window" << endl;
    else
        vect.at(m) = obj;   // Throws an exception if out of bounds
}

PacketInfo AcrdaWnd::get(int m)
{
    return vect.at(m);
}

const PacketInfo AcrdaWnd::get(int m) const
{
    return vect.at(m);
}


// -----------------------------------------------------


bool AcrdaWnd::operator==(const AcrdaWnd &other) const
{
    return (vect == other.vect) && (wndLeft == other.wndLeft);
}

bool AcrdaWnd::operator!=(const AcrdaWnd &other) const {
    return !(*this == other);
}



void AcrdaWnd::setPacketResolvedFlag(int k) {
    vect.at(k).setResolved();
}

std::vector<int> AcrdaWnd::getNewResolvableIndices()
{
    std::vector<int> newResolvableIndices;
    newResolvableIndices.reserve(size());

    for (int i=0; i<vect.size(); i++) {
        PacketInfo p = vect[i];

        // If already resolved, skip to the next packet.
        if (p.isResolved())
            continue;

        // A packet cannot be added to the window before WndLeft, since it cannot arrive before WndLeft.
        // This is also checked in the "add" methods, to be sure. Still, after a window shift it may
        // happen that a packet starts before WndLeft and ends inside the current window, so it must be
        // considered for collisions (if it was not resolved).
        // There can also be a packet whose reception ends past the end of the window.
        // These packets must be considered for collisions, but they can't be resolved, so we skip them.
        if ((p.getEndTime() > wndLeft + wndLength) || (p.getStartTime() < wndLeft))
            continue;

        simtime_t start = p.getStartTime();
        simtime_t end   = p.getEndTime();
        double snr0 = p.getSnr();   // SNR of the current packet (linear)
        double denom = 1;   // Denominator to evaluate SINR of the current packet: it depends
                            // on the noise, and interference from other packets

        // Assume it to be resolvable
        bool isThisResolvable = true;
        for (int j=0; j<vect.size() && isThisResolvable; j++) {
            if (j!=i) {
                PacketInfo p2 = vect[j];
                if (p2.isResolved())  // if the candidate conflicting pkt is resolved, it cannot collide
                    continue;
                simtime_t start2 = p2.getStartTime();
                simtime_t end2   = p2.getEndTime();

                // If p2 and p are not resolved and are colliding, we check the SINR.
                // Then we skip to the next packet i (outer for loop).
                bool expr1 = (start <= start2 && start2 < end);
                bool expr2 = (start < end2 && end2 <= end);
                bool expr3 = (start2 <= start && end <= end2);
                if (expr1 || expr2 || expr3) {
                    double overlapFactor;
                    if (expr1)
                        overlapFactor = (end - start2) / (end - start);
                    else if (expr2)
                        overlapFactor = (end2 - start) / (end - start);
                    else //if (expr3)
                        overlapFactor = 1;

                    denom += overlapFactor + p2.getSnr();

                    // TODO Stop immediately if we see that we have fallen below threshold: isThisResolvable serves this purpose
                }
            }
        }

        //std::cout << snr0 << " " << denom << " " << sinrThresh << endl;
        if (snr0 / denom < sinrThresh) // Everything must be linear!
            isThisResolvable = false;

        if (isThisResolvable)
            newResolvableIndices.push_back(i);
    }

    newResolvableIndices.shrink_to_fit();
    return newResolvableIndices;
}


//void AcrdaWnd::updateAllResolvedFlags()
//{
//    std::vector<int> newResIdx = getNewResolvableIndices();
//    for (int i=0; i<newResIdx.size(); i++)
//        vect[newResIdx[i]].setResolved();
//}



int AcrdaWnd::getNumResolvedPkts() //based on flags
{
    int nres = 0;
    for (int i=0; i<vect.size(); i++)
        if (vect[i].isResolved())
            nres++;
    return nres;
}


std::vector<PacketInfo> AcrdaWnd::getResolvedPkts()
{
    std::vector<PacketInfo> resolvedPkts;
    resolvedPkts.reserve(size());

    for (int i=0; i<vect.size(); i++)
        if (vect[i].isResolved())
            resolvedPkts.push_back(vect[i]);

    resolvedPkts.shrink_to_fit();
    return resolvedPkts;
}



void AcrdaWnd::updateResolvedFlagsOfReplicas()
{
    for (int i=0; i<vect.size(); i++) {
        PacketInfo p = vect[i];

        // If not resolved, skip to the next packet.
        if (! p.isResolved())
            continue;

        // Loop through all other packets to flag all replicas as resolved
        for (int j=0; j<vect.size(); j++) {
            if (i!=j) {
                if (vect[j].isReplicaOf(&p) && vect[j].isResolved()==false)
                    vect[j].setResolved();
            }
        }
    }
}

bool AcrdaWnd::areAllResolved()
{
    return (getNumResolvedPkts() == size());
}



void AcrdaWnd::shift(double newWndLeft)
{
    ASSERT(newWndLeft >= wndLeft.dbl());
    wndLeft = newWndLeft;

    // Remove old packets
    int j = 0;
    int i = 0;
    while (j<vect.size()) {
        if (vect[j].getEndTime() >= wndLeft)
            vect[i++] = vect[j];
        j++;
    }

    // Now i is the index of the first free slot, i.e. the actual size: we need to actually delete those objects.
    vect.resize(i);
}



std::string AcrdaWnd::toString()
{
    std::ostringstream strStream;
    strStream << "\n------\nCurrent window (" << size() << "elements):\n";
    for (int i=0; i<size(); i++) {
        PacketInfo *p = & vect[i];
        strStream << "hostID=" << p->getHostIdx() << "\t";
        strStream << "pkID=" << p->getPkIdx() << "\t";
        strStream << p->getStartTime() << " to " << p->getEndTime();
        strStream << "  -  " << p->isResolved();
        strStream << endl;
    }
    strStream << "------\n";
    return strStream.str();
}
