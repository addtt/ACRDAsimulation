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
    explicit AcrdaWnd();

    /**
     * Destructor. The contained objects that were owned by the container
     * will be deleted.
     */
    virtual ~AcrdaWnd();


    /** @name Container functions. */
    //@{

    /**
     * Returns the index of last used position+1. This only equals the
     * number of contained objects if there are no "holes" (NULL elements)
     * in the array.
     */
    int size() const {return vect.size();}

    /**
     * Makes the container empty. Contained objects that were owned by the
     * container will be deleted.
     */
    void clear();


    /**
     * Inserts the object into the array. Only the pointer of the object
     * will be stored. The return value is the object's index in the
     * array.
     */
    void add(PacketInfo obj);

    /**
     * Inserts the object into the array at the given position. If
     * the position is occupied, the function throws a cRuntimeError.
     * The return value is the object's index in the array.
     */
    void addAt(int m, PacketInfo obj);


    /**
     * Returns reference to the mth object in the array. Returns NULL
     * if the mth position is not used.
     */
    PacketInfo get(int m);


    /**
     * Returns reference to the mth object in the array. Returns NULL
     * if the mth position is not used.
     */
    const PacketInfo get(int m) const;

    // -----------------------------------------------------

    int firstResolvableIndex();

    PacketInfo firstResolvable();

    void updateAllResolvedFlags();

    int getNumResolved();

    void shift(double newWndLeft);

    simtime_t getWndLeft() { return wndLeft;}

    std::string toString();


};

#endif /* ACRDAWND_H_ */
