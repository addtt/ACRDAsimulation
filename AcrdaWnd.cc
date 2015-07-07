/*
 * AcrdaWnd.cc
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#include <AcrdaWnd.h>

#include <string.h>
#include <algorithm>
#include <sstream>
#include "globals.h"
#include "cexception.h"
#include <sstream>


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
    vect.push_back(obj);
}

void AcrdaWnd::addAt(int m, PacketInfo obj)
{
    vect.at(m) = obj;
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




std::vector<int> AcrdaWnd::getNewResolvableIndices(bool firstOnly)
{
    std::vector<int> newResolvableIndices;
    newResolvableIndices.reserve(size());

    for (int i=0; i<vect.size(); i++) {
        PacketInfo p = vect[i];

        // If already resolved, skip to the next packet.
        if (p.isResolved())
            continue;

        simtime_t start = p.getStartTime();
        simtime_t end   = p.getEndTime();

        // Assume it to be resolvable
        bool isThisResolvable = true;
        for (int j=0; j<vect.size() && isThisResolvable; j++) {
            if (j!=i) {
                PacketInfo p2 = vect[j];
                if (p2.isResolved())  // if the candidate conflicting pkt is resolved, it cannot collide
                    continue;
                simtime_t start2 = p2.getStartTime();
                simtime_t end2   = p2.getEndTime();

                // If p2 and p are not resolved and are colliding, then they cannot be resolved for now,
                // so we skip to the next packet i (outer for loop).
                if ((start <= start2 && start2 < end) || (start < end2 && end2 <= end))
                    isThisResolvable = false;
            }
        }

        if (isThisResolvable) {
            newResolvableIndices.push_back(i);
            if (firstOnly)
                break;
        }
    }

    newResolvableIndices.shrink_to_fit();
    return newResolvableIndices;
}


void AcrdaWnd::updateAllResolvedFlags()
{
    std::vector<int> newResIdx = getNewResolvableIndices();
    for (int i=0; i<newResIdx.size(); i++)
        vect[newResIdx[i]].setResolved();
}


int AcrdaWnd::firstResolvableIndex()
{
    std::vector<int> newResIdx = getNewResolvableIndices(true);
    bool isThereNewResIdx = (newResIdx.size() > 0);
    return isThereNewResIdx ? newResIdx[0] : (-1);
}

PacketInfo AcrdaWnd::firstResolvable()
{
    return vect.at(firstResolvableIndex());
}


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
                if (vect[j].isReplicaOf(&p))
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

    updateResolvedFlagsOfReplicas();
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
