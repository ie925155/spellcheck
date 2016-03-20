/**
 * File: cvector.h
 * ---------------
 * Defines the interface for the CVector type.
 *
 * The CVector manages a linear, indexed collection of homogeneous elements.
 * Think of it as an upgrade from the raw C array, with convenient dynamic
 * memory management (grow/shrink), operations to insert/remove, and
 * sort and search facilities. In order to work for all types of elements,
 * the size of each element is specified when creating the CVector, and
 * all CVector elements must be passed and returned via void* pointers.
 * Given its extensive use of untyped pointers, the CVector is a bit tricky
 * to use correctly as a client. Be diligent!
 *
 * CS107 Fall 2011 jzelenski
 */

#ifndef _cvector_h
#define _cvector_h

#include <stdbool.h>	//  this header introduces the C99 bool type

/**
 * Type: CVectorCmpElemFn
 * ----------------------
 * CVectorCmpElemFn is the typename for a pointer to a client-supplied
 * comparator function. Such functions are passed to CVector to sort or
 * search for elements.  The comparator takes two const void* pointers,
 * each of which points to an element of the type stored in the CVector,
 * and returns an integer. That integer should indicate the ordering of
 * the two elements using the same convention as the strcmp library function:
 *
 *   If element at elemAddr1 < element at elemAddr2, return a negative number
 *   If element at elemAddr1 > element at elemAddr2, return a positive number
 *   If element at elemAddr1 = element at elemAddr2, return zero
 */

typedef int (*CVectorCmpElemFn)(const void *elemAddr1, const void *elemAddr2);


/**
 * Type: CVectorCleanupElemFn
 * --------------------------
 * CVectorCleanupElemFn is the typename for a pointer to a client-supplied
 * cleanup function. Such function pointers are passed to CVectorCreate to
 * apply to an element when it is removed from the CVector. The cleanup
 * function takes one void* pointer that points to the element being removed.
 */

typedef void (*CVectorCleanupElemFn)(void *elemAddr);

/**
 * Type: CVectorMapElemFn
 * ----------------------
 * CVectorMapElemFn is the typename for a pointer to a client-supplied mapping
 * function. Such functions are passed to CVectorMap to apply to each element
 * in in the CVector. The mapping function takes a void* pointer to an element
 * and another void* for extra data passed by the client.
 */

typedef void (*CVectorMapElemFn)(void *elemAddr, void *auxData);



/**
 * Type: CVector
 * -------------
 * Defines the CVector type. The type is "incomplete", i.e. deliberately
 * avoids stating the field names/types for the struct CVectorImplementation.
 * (That struct is completed in the implementation code not visible to
 * clients). The incomplete type forces the client to respect the privacy
 * of the representation. Client declare variables only of type CVector *
 * (pointers only, never of the actual struct) and cannot never dereference
 * a CVector * nor attempt to read/write its internal fields. A CVector
 * is manipulated solely through the functions listed in this interface
 * (this is analogous to how you use a FILE *).
 */
typedef struct CVectorImplementation CVector;


/**
 * Function: CVectorCreate
 * Usage: CVector *myvec = CVectorCreate(sizeof(int), 10, NULL);
 * -------------------------------------------------------------
 * Creates a new empty CVector and returns a pointer to it. The pointer
 * points to storage allocated in the heap. When done with this CVector, the
 * client should call CVectorDispose to dispose of it. If allocation fails, an
 * assert is raised.
 *
 * The elemSize parameter specifies the size, in bytes, of the elements that
 * will be stored in the CVector. For example, to store elements of type
 * double, pass sizeof(double) for the elemSize. Note that all elements stored
 * in a given CVector must be of the same type. An assert is raised if elemSize
 * is not greater than zero.
 *
 * The capacityHint parameter allows the client to tune the resizing behavior
 * for their needs. The CVector's internal storage will initially be allocated
 * to hold this number of elements. This capacityHint is not a binding limit,
 * just a hint.  If the initial allocation is outgrown, the storage capacity
 * will double in size.  If planning to store many elements, specifying a large
 * capacityHint will result in an appropriately large initial allocation and
 * fewer resizing operations later.  For a small vector, a small capacityHint
 * will result in several smaller allocations and potentially less waste.  If
 * capacityHint is 0, an internal default value is used. An assert is raised if
 * capacityHint is negative.
 *
 * The cleanupFn is a client callback that will be called on an element being
 * removed/replaced (via CVectorRemove/CVectorReplace, respectively) and on
 * every element in the CVector when it is destroyed (via CVectorDispose). The
 * client can use this function to do any deallocation/cleanup required for the
 * element, such as freeing any memory to which the element points (if the
 * element itself is a pointer). The client can pass NULL for cleanupFn if
 * elements don't require any special cleanup.
 */

CVector *CVectorCreate(int elemSize, int capacityHint, CVectorCleanupElemFn cleanupFn);


/**
 * Function: CVectorDispose
 * Usage:  CVectorDispose(myvec);
 * ------------------------------
 * Disposes of this CVector and deallocates its memory.  It does *not*
 * automatically free memory owned by pointers embedded within the
 * elements. This would require knowledge of the structure of the elements,
 * which the CVector does not have. It *does* loop over the elements, calling
 * the cleanupFn that was supplied to CVectorCreate when the given CVector was
 * created.
 */
void CVectorDispose(CVector *v);


/**
 * Function: CVectorCount
 * Usage:  int count = CVectorCount(myvec);
 * ---------------------------------------
 * Returns the number of elements currently stored in this CVector.
 * Operates in constant-time.
 */

int CVectorCount(const CVector *v);

/**
 * Method: CVectorNth
 * Usage:  int num = *(int *)CVectorNth(myvec, 0);
 * -----------------------------------------------
 * Returns a pointer to the element stored at a given index in this CVector.
 * Valid indexes are 0 to count-1.  An assert is raised if index is out
 * of bounds.  Note this function returns a pointer into the CVector's
 * storage, so the pointer should be used with care. In particular, the pointer
 * returned by CVectorNth can become invalid after a call that adds, removes,
 * or rearranges elements within the CVector. The CVector could have been
 * designed without this direct access, but it is useful and efficient to offer
 * it, despite its potential pitfalls.  Operates in constant-time.
 */

void *CVectorNth(const CVector *v, int index);

/**
 * Function: CVectorInsert
 * Usage:  CVectorInsert(myvec, &elem, 0);
 * ---------------------------------------
 * Inserts a new element into this CVector, placing it at the given index
 * and shifting up other elements to make room. An assert is raised if index
 * is less than 0 or greater than the count. The new element is
 * copied from the memory pointed to by elemAddr. The capacity is
 * enlarged if neccssary, an assert is raied on allocation failure.
 * Operates in linear-time.
 */

void CVectorInsert(CVector *v, const void *elemAddr, int index);


/**
 * Function: CVectorAppend
 * Usage:  CVectorAppend(myvec, &elem);
 * ------------------------------------
 * Appends a new element to the end of this CVector.  The new element is
 * copied from the memory pointed to by elemAddr. The capacity is
 * enlarged if necessary, an assert is raised on allocation failure.
 * Operates in constant-time (amortized).
 */

void CVectorAppend(CVector *v, const void *elemAddr);


/**
 * Function: CVectorReplace
 * Usage:  CVectorReplace(myvec, &elem, 0);
 * ----------------------------------------
 * Overwrites the element at the given index with a new element. Before
 * being overwritten, the cleanupFn that was supplied to CVectorCreate is
 * called on the old element. Then, the new element is copied from
 * the memory pointed to by elemAddr to replace the old element at index.
 * An assert is raised if index is out of bounds. Operates in constant-time.
 */

void CVectorReplace(CVector *v, const void *elemAddr, int index);


/**
 * Function: CVectorRemove
 * Usage:  CVectorRemove(myvec, 0);
 * --------------------------------
 * Removes the element at the given index from this CVector and shifts
 * subsequent elements down to close the gap. Before the element is removed,
 * the cleanupFn that was supplied to CVectorCreate will be called on the
 * element.  An assert is raised if index is out of bounds. Operates in
 * linear-time.
 */

void CVectorRemove(CVector *v, int index);


/**
 * Function: CVectorSearch
 * Usage:  int found = CVectorSearch(myvec, &key, MyCmpElem, 0, false);
 * --------------------------------------------------------------------
 * Searches this CVector for an element matching the element to which the given
 * key points. It uses the provided comparefn callback to compare elements. The
 * search considers all elements from startIndex to end. To search the entire
 * CVector, specify a startIndex of 0. The isSorted parameter allows the client
 * to specify that elements are already stored in sorted order, in which case
 * CVectorSearch uses a faster binary search. If isSorted is false, linear
 * search is used instead.  If a match is found, the index of the matching
 * element is returned.  If not, linear search returns -1 and binary search
 * returns (-(insertion point) - 1).  The insertion point is an index at which
 * the key could be inserted into the CVector to preserve sorted order (not
 * necessarily the first such index).  This function does not re-arrange/change
 * elements within the CVector or modify the key. Operates in linear-time or
 * logarithmic-time (sorted).
 *
 * An assert is raised if startIndex is less than 0 or greater than the count
 * (although searching from count will never find anything, allowing this case
 * means client can search an empty CVector from 0 without getting an assert).
 */

int CVectorSearch(const CVector *v, const void *key, CVectorCmpElemFn comparefn, int startIndex, bool isSorted);


/**
 * Function: CVectorSort
 * Usage:  CVectorSort(myvec, MyCmpElem);
 * --------------------------------------
 * Rearranges elements in this CVector into ascending order according to the
 * provided comparator function.  Operates in NlgN-time.
 */

void CVectorSort(CVector *v, CVectorCmpElemFn comparefn);


/**
 * Function: CVectorMap
 * Usage:  CVectorMap(myvec, PrintElem, &myData);
 * ----------------------------------------------
 * Iterates over all of the elements in this CVector and calls mapfn on each
 * one.  The callback function is passed a pointer to the element and the
 * auxData pointer. The auxData value allows the client to pass extra state
 * information if desired. If no client data is needed, this argument can be
 * NULL.  The elements are processed in sequential order. Operates in linear
 * time.
 */

void CVectorMap(CVector *v, CVectorMapElemFn mapfn, void *auxData);

#endif
