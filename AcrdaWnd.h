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
    std::vector<PacketInfo> vect;    // vector of PacketInfo objects
    simtime_t wndLeft;  // Left boundary of the window
    simtime_t wndLength; // Length (width) of the window
    double sinrThresh;  // SINR threshold in linear


public:


    /**
     * Default constructor. The object AcrdaWnd is not initialized and
     * cannot be used properly since wndLength is 0.
     */
    explicit AcrdaWnd() {}

    /**
     * Explicit constructor for AcrdaWnd, with initialization of wndLength.
     */
    explicit AcrdaWnd(double wndLength) : wndLength (wndLength)
    {
    }

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
     * Returns the length of the window (in time).
     */
    simtime_t length() const {return wndLength;}

    /**
     * Makes the container empty.
     */
    void clear();


    /**
     * Appends the object into the window.
     *
     * It does nothing if the packet's start time is smaller then the left
     * boundary (beginning) of the window.
     */
    void add(PacketInfo obj);

    /**
     * Inserts the object into the window at the given position. If
     * the position is occupied, the function overwrites the object.
     * If the index is not valid, an out of bounds exception is thrown.
     *
     * It does nothing if the packet's start time is smaller then the left
     * boundary (beginning) of the window.
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
     * Sets the packet at index k in the window as resolved, calling the method
     * setResolved() in PacketInfo.
     */
    void setPacketResolvedFlag(int k);


    /**
     * Checks which packets in the window are not resolved but can be immediately resolved
     * (because they only overlap with already resolved packets). It returns a vector with
     * the indices of all such packets. If no such packet exists, i.e. all packets collide
     * with some other non-resolved packet, the output vector has size 0.
     */
    std::vector<int> getNewResolvableIndices();


//    /**
//     * Visits all packets in the current window and flags as resolvable all packets
//     * that are currently resolvable, i.e. that do not collide with other non-resolved
//     * packets.
//     *
//     * It checks resolvable packets using the method getNewResolvableIndices(), and flags
//     * them as resolved.
//     */
//    void updateAllResolvedFlags();



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
     * Shifts the window so that the left boundary is newWndLeft. Old packets are removed, and the
     * internal vector is defragmented. Note that this method keeps all packets whose start time
     * is outside the window and whose end time is inside the window.
     * The parameter newWndLeft must be greater than or equal to the current left boundary of the window.
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
