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
    double sinrThresh;   // SINR threshold in linear


public:


    /**
     * Constructor.
     */
    explicit AcrdaWnd() {}

    /**
     * Destructor. The contained objects that were owned by the container
     * will be deleted.
     */
    virtual ~AcrdaWnd();


    // ----------------------------------------


    bool operator==(const AcrdaWnd &other) const;
    bool operator!=(const AcrdaWnd &other) const;

    /**
     * Returns the size of the window, i.e. the number of packets currently stored.
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
     * Returns the threshold for the SINR, in linear.
     */
    double getSinrThresh() const {
        return sinrThresh;
    }

    /**
     * Sets the linear threshold for the SINR.
     */
    void setSinrThresh(double sinrThresh) {
        this->sinrThresh = sinrThresh;    // SINR threshold in linear
    }



    /**
     * Checks which packets in the window are not resolved but can be immediately resolved
     * (because they only overlap with already resolved packets). It returns a vector with
     * the indices of all such packets. If no such packet exists, i.e. all packets collide
     * with some other non-resolved packet, the output vector has size 0.
     *
     * If the parameter firstOnly is true, then the algorithm stops as soon as one new
     * resolvable packet is found, and the returned vector has size 1 at most. This parameter
     * is false by default.
     */
    std::vector<int> getNewResolvableIndices(bool firstOnly=false);


//    /**
//     * Visits all packets in the current window and flags as resolvable all packets
//     * that are currently resolvable, i.e. that do not collide with other non-resolved
//     * packets.
//     *
//     * It checks resolvable packets using the method getNewResolvableIndices(), and flags
//     * them as resolved.
//     */
//    void updateAllResolvedFlags();


//    /**
//     * Returns the index of the first resolvable (and not yet resolved) packet, i.e. the
//     * first packet that does not collide with non-resolved packets. It returns -1 if no
//     * such packet exists.
//     */
//    int firstResolvableIndex();
//
//
//    /**
//     * Returns the fist resolvable (and not yet resolved) packet, i.e. the first packet that
//     * does not collide with non-resolved packets. It throws an exception if no such packet
//     * exists.
//     */
//    PacketInfo firstResolvable();


    /**
     * Loops through all resolved packets in the window, and flags all of its replicas as resolved.
     */
    void updateResolvedFlagsOfReplicas();

    /**
     * Returns the number of packets in the window that are flagged as resolved.
     */
    int getNumResolvedPkts();

    /**
     * Returns the packets in the window that are flagged as resolved.
     */
    std::vector<PacketInfo> getResolvedPkts();

    /**
     * Returns true if all packets of the window are flagged as resolved.
     */
    bool areAllResolved();

    /**
     * Shifts the window so that the left boundary is newWndLeft. Old packets are removed, the internal
     * vector is defragmented.
     * Note that newWndLeft must be greater than or equal to the current left boundary of the window.
     */
    void shift(double newWndLeft);

    /**
     * Returns the leftmost boundary of the window.
     */
    simtime_t getWndLeft() { return wndLeft;}

    /**
     * Returns a string representation of the window.
     */
    std::string toString();

};

#endif /* ACRDAWND_H_ */
