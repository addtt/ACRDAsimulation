/*
 * AcrdaWnd.h
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#ifndef ACRDAWND_H_
#define ACRDAWND_H_

#include <cobject.h>
#include <omnetpp.h>

class AcrdaWnd : cObject {

public:
    /**
     * Walks along a AcrdaWnd.
     */
    class Iterator
    {
      private:
        AcrdaWnd *array;
        int k;

      public:
        /**
         * Constructor. Iterator will walk on the array passed as argument.
         * The starting object will be the first (if athead==true) or
         * the last (athead==false) object in the array, not counting empty slots.
         */
        Iterator(const AcrdaWnd& a, bool athead=true)  {init(a, athead);}

        Iterator()  {}

        /**
         * Reinitializes the iterator object.
         */
        void init(const AcrdaWnd& a, bool athead=true);

        void init(bool athead=true);

        /**
         * Returns the current object, or NULL if the iterator is not
         * at a valid position.
         */
        cObject *currElement()  {return array->get(k);}

        int currIndex()  {return k;}

        /**
         * Returns true if the iterator has reached either end of the array.
         */
        bool end() const   {return k<0 || k>=array->size();}

        /**
         * Returns the current object, then moves the iterator to the next item.
         * Empty slots in the array are skipped.
         * If the iterator has reached either end of the array, nothing happens;
         * you have to call init() again to restart iterating.
         * If elements are added or removed during interation, the behaviour
         * is undefined.
         */
        cObject *operator++(int);

        /**
         * Returns the current object, then moves the iterator to the previous item.
         * Empty slots in the array are skipped.
         * If the iterator has reached either end of the array, nothing happens;
         * you have to call init() again to restart iterating.
         * If elements are added or removed during interation, the behaviour
         * is undefined.
         */
        cObject *operator--(int);
    };

private:
    cObject **vect;    // vector of objects
    int capacity;      // allocated size of vect[]
    int delta;         // if needed, grows by delta
    int firstfree;     // first free position in vect[]
    int last;          // last used position
    simtime_t wndLeft; // Left boundary of the window

private:
    void copy(const AcrdaWnd& other);

public:
    /** @name Constructors, destructor, assignment. */
    //@{


    /**
     * Constructor. The initial capacity of the container and the delta
     * (by which the capacity will grow if it gets full) can be specified.
     */
    explicit AcrdaWnd(int delta=10);

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
    int size() const {return last+1;}

    /**
     * Makes the container empty. Contained objects that were owned by the
     * container will be deleted.
     */
    void clear();

    /**
     * Returns the allocated size of the underlying array.
     */
    int getCapacity() const {return capacity;}

    /**
     * Resizes the the underlying array, without changing the contents of
     * this array. The specified capacity cannot be less than size().
     */
    void setCapacity(int capacity);

    /**
     * Inserts the object into the array. Only the pointer of the object
     * will be stored. The return value is the object's index in the
     * array.
     */
    int add(cObject *obj);

    /**
     * Inserts the object into the array at the given position. If
     * the position is occupied, the function throws a cRuntimeError.
     * The return value is the object's index in the array.
     */
    int addAt(int m, cObject *obj, bool overwrite=false);


    /**
     * Searches the array for the pointer of the object passed and returns
     * the index of the first match. If the object was not found, -1 is
     * returned.
     */
    int find(cObject *obj) const;


    /**
     * Returns reference to the mth object in the array. Returns NULL
     * if the mth position is not used.
     */
    cObject *get(int m);


    /**
     * Returns reference to the mth object in the array. Returns NULL
     * if the mth position is not used.
     */
    const cObject *get(int m) const;


    /**
     * The same as get(int). With the indexing operator,
     * AcrdaWnd can be used as a vector.
     */
    cObject *operator[](int m)  {return get(m);}


    /**
     * The same as get(int). With the indexing operator,
     * AcrdaWnd can be used as a vector.
     */
    const cObject *operator[](int m) const  {return get(m);}


    /**
     * Returns true if position m is used in the array, otherwise false.
     */
    bool exist(int m) const  {return m>=0 && m<=last && vect[m]!=NULL;}


    /**
     * Removes the object given with its index/name/pointer from the
     * container. (If the object was owned by the container, drop()
     * is called.)
     */
    cObject *remove(int m);


    /**
     * Removes the object given with its index/name/pointer from the
     * container, and returns the same pointer. If the object was not
     * found, NULL is returned. (If the object was owned by the container, drop()
     * is called.)
     */
    cObject *remove(cObject *obj);
    //@}


    // -----------------------------------------------------

    int firstResolvableIndex();

    cObject *firstResolvable();

    void updateAllResolvedFlags();

    int getNumResolved();

    void shift(double newWndLeft);

    void defragment();

    simtime_t getWndLeft() { return wndLeft;}


};

#endif /* ACRDAWND_H_ */
