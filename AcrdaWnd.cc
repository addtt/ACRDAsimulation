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


int AcrdaWnd::firstResolvableIndex()
{
    for (int i=0; i<vect.size(); i++) {
        PacketInfo p = vect[i];

        // If already resolved, skip to the next packet.
        if (p.isResolved())
            continue;

        simtime_t start = p.getStartTime();
        simtime_t end   = p.getEndTime();

        // Assume it to be resolvable
        bool resolvable = true;
        for (int j=0; j<vect.size() && resolvable; j++) {
            if (j!=i) {
                PacketInfo p2 = vect[j];
                if (p2.isResolved())  // if the candidate conflicting pkt is resolved, it cannot collide
                    continue;
                simtime_t start2 = p2.getStartTime();
                simtime_t end2   = p2.getEndTime();

                // If p2 and p are not resolved and are colliding, then they cannot be resolved for now,
                // so we skip to the next packet i (outer for loop).
                if ((start <= start2 && start2 < end) || (start < end2 && end2 <= end))
                    resolvable = false;
            }
        }

        // This is the first resolvable packet in the window array!
        if (resolvable)
            return i;
    }

    return -1;
}

PacketInfo AcrdaWnd::firstResolvable()
{
    return vect.at(firstResolvableIndex());
}


int AcrdaWnd::getNumResolved() //based on flags
{
    int nres = 0;
    for (int i=0; i<vect.size(); i++)
        if (vect[i].isResolved())
            nres++;
    return nres;
}



void AcrdaWnd::updateAllResolvedFlags()
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

    updateAllResolvedFlags();
}



std::string AcrdaWnd::toString()
{
    std::ostringstream convert;
    convert << "\n------\nCurrent window (" << size() << "elements):\n";
    for (int i=0; i<size(); i++) {
        PacketInfo *p = & vect[i];
        convert << "hostID=" << p->getHostIdx() << "\t";
        convert << "pkID=" << p->getPkIdx() << "\t";
        convert << p->getStartTime() << " to " << p->getEndTime();
        convert << "  -  " << p->isResolved();
        convert << endl;
    }
    convert << "------\n";
    return convert.str();
}
