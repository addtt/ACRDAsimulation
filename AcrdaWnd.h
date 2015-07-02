/*
 * AcrdaWnd.h
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#ifndef ACRDAWND_H_
#define ACRDAWND_H_

#include <omnetpp.h>
#include "PacketInfo.h"

class AcrdaWnd {


private:
    std::vector<PacketInfo> vect;    // vector of objects
    simtime_t wndLeft; // Left boundary of the window


public:
    /** @name Constructors, destructor, assignment. */
    //@{


    /**
     * Constructor.
     */
    explicit AcrdaWnd() {}

    /**
     * Destructor. The contained objects that were owned by the container
     * will be deleted.
     */
    virtual ~AcrdaWnd();


    /** @name Container functions. */
    //@{

    /**
     * Returns the size of the window.
     */
    int size() const {return vect.size();}

    /**
     * Makes the container empty.
     */
    void clear();


    /**
     * Appends the object into the window.
     */
    void add(PacketInfo obj);

    /**
     * Inserts the object into the window at the given position. If
     * the position is occupied, the function overwrites the object.
     * If the index is not valid, an out of bounds exception is thrown.
     */
    void addAt(int m, PacketInfo obj);


    /**
     * Returns the m-th object in the window. If the index is not valid,
     * an out of bounds exception is thrown.
     */
    PacketInfo get(int m);


    /**
     * Returns the m-th object in the window. If the index is not valid,
     * an out of bounds exception is thrown.
     */
    const PacketInfo get(int m) const;

    // -----------------------------------------------------

    /**
     * Visits all packets in the current window and flags as resolvable all packets
     * that are currently resolvable, i.e. that do not collide with other non-resolved
     * packets.
     */
    void updateAllResolvedFlags();

    int firstResolvableIndex();

    PacketInfo firstResolvable();

    void updateResolvedFlagsOfReplicas();

    int getNumResolved();

    void shift(double newWndLeft);

    simtime_t getWndLeft() { return wndLeft;}

    std::string toString();


};

#endif /* ACRDAWND_H_ */
