/*
 * AcrdaWnd.cc
 *
 *  Created on: Jun 25, 2015
 *      Author: andrea
 */

#include <AcrdaWnd.h>

#include <string.h>  // memcmp, memcpy, memset
#include <algorithm>  // min, max
#include <sstream>
#include "globals.h"
#include "cexception.h"
#include "PacketInfo.h"


//Register_Class(AcrdaWnd);

void AcrdaWnd::Iterator::init(const AcrdaWnd& a, bool athead)
{
    array = const_cast<AcrdaWnd *>(&a); // we don't want a separate Const_Iterator class

    if (athead)
    {
        // fast-forward to first non-empty slot
        // (Note: we exploit that get(k) just returns NULL when k is out of bounds)
        k = 0;
        while (!array->get(k) && k < array->size())
            k++;

    }
    else
    {
        // rewind to first non-empty slot
        k = array->size()-1;
        while (!array->get(k) && k>=0)
            k--;
    }
}

void AcrdaWnd::Iterator::init(bool athead)
{
    if (!array)
        return;

    if (athead)
    {
        // fast-forward to first non-empty slot
        // (Note: we exploit that get(k) just returns NULL when k is out of bounds)
        k = 0;
        while (!array->get(k) && k < array->size())
            k++;

    }
    else
    {
        // rewind to first non-empty slot
        k = array->size()-1;
        while (!array->get(k) && k>=0)
            k--;
    }
}

cObject *AcrdaWnd::Iterator::operator++(int)
{
    if (k<0 || k>=array->size())
        return NULL;
    cObject *obj = array->get(k);

    k++;
    while (!array->get(k) && k<array->size())
        k++;
    return obj;
}

cObject *AcrdaWnd::Iterator::operator--(int)
{
    if (k<0 || k>=array->size())
        return NULL;
    cObject *obj = array->get(k);
    k--;
    while (!array->get(k) && k>=0)
        k--;
    return obj;
}


//----


AcrdaWnd::AcrdaWnd(int dt) : cObject()
{
    delta = std::max(1,dt);
    firstfree = 0;
    last = -1;
    wndLeft = 0;
    vect = new cObject *[capacity];
    for (int i=0; i<capacity; i++)
        vect[i] = NULL;
}

AcrdaWnd::~AcrdaWnd()
{
    clear();
    delete [] vect;
}

void AcrdaWnd::copy(const AcrdaWnd& other)
{
    capacity = other.capacity;
    delta = other.delta;
    firstfree = other.firstfree;
    last = other.last;
    wndLeft = other.wndLeft;
    delete [] vect;
    vect = new cObject *[capacity];
    memcpy(vect, other.vect, capacity * sizeof(cObject *));

    for (int i=0; i<=last; i++)
        if (vect[i])
            vect[i] = vect[i]->dup();
}



void AcrdaWnd::clear()
{
    for (int i=0; i<=last; i++)
    {
        cObject *obj = vect[i];
        if (obj) {
            delete obj;
            vect[i] = NULL;  // this is not strictly necessary
        }
    }
    firstfree = 0;
    last = -1;
    wndLeft = 0;
}

void AcrdaWnd::setCapacity(int newCapacity)
{
    if (newCapacity < size())
        throw cRuntimeError(this,"setCapacity: new capacity %d cannot be less than current size %d", newCapacity,size());

    cObject **newVect = new cObject *[newCapacity];
    for (int i=0; i<=last; i++)
        newVect[i] = vect[i];
    for (int i=last+1; i<capacity; i++)
        newVect[i] = NULL;
    delete [] vect;
    vect = newVect;
    capacity = newCapacity;
}

int AcrdaWnd::add(cObject *obj)
{
    if (!obj)
        throw cRuntimeError(this,"cannot insert NULL pointer");

    int retval;
    if (firstfree < capacity)  // fits in current vector
    {
        vect[firstfree] = obj;
        retval = firstfree;
        last = std::max(last,firstfree);
        do {
            firstfree++;
        } while (firstfree<=last && vect[firstfree]!=NULL);
    }
    else // must allocate bigger vector
    {
        cObject **v = new cObject *[capacity+delta];
        memcpy(v, vect, sizeof(cObject*)*capacity );
        memset(v+capacity, 0, sizeof(cObject*)*delta);
        delete [] vect;
        vect = v;
        vect[capacity] = obj;
        retval = last = capacity;
        firstfree = capacity+1;
        capacity += delta;
    }
    return retval;
}

int AcrdaWnd::addAt(int m, cObject *obj, bool overwrite)
{
    if (!obj)
        throw cRuntimeError(this,"cannot insert NULL pointer");

    if (m<capacity)  // fits in current vector
    {
        if (m<0)
            throw cRuntimeError(this,"addAt(): negative position %d",m);
        if (vect[m]!=NULL && !overwrite)
            throw cRuntimeError(this,"addAt(): position %d already used",m);
        vect[m] = obj;
        last = std::max(m,last);
        if (firstfree==m)
            do {
                firstfree++;
            } while (firstfree<=last && vect[firstfree]!=NULL);
    }
    else // must allocate bigger vector
    {
        cObject **v = new cObject *[m+delta];
        memcpy(v, vect, sizeof(cObject*)*capacity);
        memset(v+capacity, 0, sizeof(cObject*)*(m+delta-capacity));
        delete [] vect;
        vect = v;
        vect[m] = obj;
        capacity = m+delta;
        last = m;
        if (firstfree==m)
            firstfree++;
    }
    return m;
}

int AcrdaWnd::find(cObject *obj) const
{
    int i;
    for (i=0; i<=last; i++)
        if (vect[i]==obj)
            return i;
    return -1;
}

cObject *AcrdaWnd::get(int m)
{
    if (m>=0 && m<=last)
        return vect[m];
    else
        return NULL;
}

const cObject *AcrdaWnd::get(int m) const
{
    if (m>=0 && m<=last)
        return vect[m];
    else
        return NULL;
}

cObject *AcrdaWnd::remove(cObject *obj)
{
    if (!obj) return NULL;

    int m = find( obj );
    if (m==-1)
        return NULL;
    return remove(m);
}

cObject *AcrdaWnd::remove(int m)
{
    if (m<0 || m>last || vect[m]==NULL)
        return NULL;

    cObject *obj = vect[m]; vect[m] = NULL;
    firstfree = std::min(firstfree, m);
    if (m==last)
        do {
            last--;
        } while (last>=0 && vect[last]==NULL);
    return obj;
}


// -----------------------------------------------------


int AcrdaWnd::firstResolvableIndex()
{
    for (int i=0; i<=last; i++) {
        if (vect[i]) {
            PacketInfo *p = (PacketInfo *) vect[i];

            // If already resolved, skip to the next packet.
            if (p->isResolved())
                continue;

            simtime_t start = p->getStartTime();
            simtime_t end   = p->getEndTime();

            // Assume it to be resolvable
            bool resolvable = true;
            for (int j=0; j<=last && resolvable; j++) {
                if (vect[j] && j!=i) {
                    PacketInfo *p2 = (PacketInfo *) vect[j];
                    if (p2->isResolved())  // if the candidate conflicting pkt is resolved, it cannot collide
                        continue;
                    simtime_t start2 = p2->getStartTime();
                    simtime_t end2   = p2->getEndTime();

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
    }
    return -1;
}

cObject *AcrdaWnd::firstResolvable()
{
    return this->get(firstResolvableIndex());
}


int AcrdaWnd::getNumResolved() //based on flags
{
    int nres = 0;
    for (int i=0; i<=last; i++)
        if (vect[i] && ((PacketInfo *) vect[i])->isResolved())
            nres++;
    return nres;
}



void AcrdaWnd::updateAllResolvedFlags()
{
    for (int i=0; i<=last; i++) {
        if (vect[i]) {
            PacketInfo *p = (PacketInfo *) vect[i]; // TODO: put this outside and then check if p is NULL (do this in other functions too)

            // If not resolved, skip to the next packet.
            if (! p->isResolved())
                continue;

            // Loop through all other packets to flag all replicas as resolved
            for (int j=0; j<=last; j++) {
                if (vect[j] && i!=j) {
                    PacketInfo *p2 = (PacketInfo *) vect[j];
                    if (p2->isReplicaOf(p))
                        p2->setResolved();
                }
            }
        }
    }
}



void AcrdaWnd::shift(double newWndLeft)
{
    ASSERT(newWndLeft >= wndLeft.dbl());
    wndLeft = newWndLeft;

    // Remove old packets
    for (int i=0; i<=last; i++) {
        if (vect[i]) {
            PacketInfo *p = (PacketInfo *) vect[i];
            if (p->getEndTime() < wndLeft)
                remove(i);
        }
    }

    defragment();

    updateAllResolvedFlags();
}


void AcrdaWnd::defragment()
{
    int j = 0;
    int i;
    for (i=0; i<=last; i++) {
        if (vect[i] && i!=j)
            addAt(j++, vect[i], true);
    }
    for (; j<=last; j++)
        remove(j);
}
