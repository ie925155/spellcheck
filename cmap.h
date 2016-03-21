/**
 * File: cmap.h
 * ------------
 * Defines the interface for the CMap type.
 *
 * The CMap manages a collection of key-value pairs or "entries". The keys
 * are always of type char *, but the values can be of any desired type. The
 * main operations are to associate a value with a key and to retrieve
 * the value associated with a key.  In order to work for all types of values,
 * all CMap values must be passed and returned via (void *) pointers.
 * Given its extensive use of untyped pointers, the CMap is a bit tricky
 * to use correctly as a client. Be diligent!
 *
* CS107 Fall 2011 jzelenski
 */

#ifndef _cmap_h
#define _cmap_h

/**
 * Type: CMapMapEntryFn
 * --------------------
 * CMapMapEntryFn is the typename for a pointer to a client-supplied mapping
 * function. Such function pointers are passed to CMapMap to apply to each
 * (key, value) pair in the CMap. The mapping function takes a char* key,
 * a void* pointer to its associated value, and another void* for
 * extra data passed by the client.
 */

typedef void (*CMapMapEntryFn)(const char *key, void *valueAddr, void *auxData);


/**
 * Type: CMapCleanupValueFn
 * ------------------------
 * CMapCleanupValueFn is the typename for a pointer to a client-supplied
 * cleanup function. Such function pointers are passed to CMapCreate to apply
 * to a value when it is removed from the CMap. The cleanup function takes one
 * void* pointer that points to the value about to be removed.
 */

typedef void (*CMapCleanupValueFn)(void *valueAddr);


/**
 * Type: CMap
 * ----------
 * Defines the CMap type. The type is "incomplete", i.e. deliberately
 * avoids stating the field names/types for the struct CMapImplementation.
 * (That struct is completed in the implementation code not visible to
 * clients). The incomplete type forces the client to respect the privacy
 * of the representation. Client declare variables only of type CMap *
 * (pointers only, never of the actual struct) and cannot never dereference
 * a CMap * nor attempt to read/write its internal fields. A CMap
 * is manipulated solely through the functions listed in this interface
 * (this is analogous to how you use a FILE *).
 */
typedef struct CMapImplementation CMap;


/**
 * Function: CMapCreate
 * Usage: CMap *mymap = CMapCreate(sizeof(int), 10, NULL);
 * ------------------------------------------------------
 * Creates a new empty CMap and returns a pointer to it. The pointer
 * points to storage allocated in the heap. When done with this CMap, the
 * client must call CMapDispose to dispose of it. If allocation fails, an
 * assert is raised.
 *
 * The valueSize parameter specifies the size, in bytes, of the
 * values that will be stored in the CMap. For example, to store values of
 * type double, pass sizeof(double) for the valueSize. Note that all
 * values stored in a given CMap must be of the same type. An assert is raised
 * if valueSize is not greater than zero.
 *
 * The capacityHint parameter allows the client to tune the resizing behavior
 * for their needs. The CMap's internal storage will initially be optimized to
 * hold this number of entries. This capacityHint is not a binding limit, just
 * a hint.  If the initial allocation is outgrown, the storage capacity will
 * double in size.  If planning to store many elements, specifying a large
 * capacityHint will result in an appropriately large initial allocation and
 * fewer resizing operations later.  For a small map, a small capacityHint will
 * result in several smaller allocations and potentially less waste.  If
 * capacityHint is 0, an internal default value is used. An assert is raised if
 * capacityHint is negative.
 *
 * The cleanupFn is a client callback that will be called on a value being
 * removed/replaced (via CMapRemove/CMapPut, respectively) and on every value
 * in the CMap when it is destroyed (via CMapDispose). The client can use this
 * function to do any deallocation/cleanup required for the value, such as
 * freeing memory to which the value points (if the value is a pointer). The
 * client can pass NULL for cleanupFn if values don't require any special
 * cleanup.
 */

CMap *CMapCreate(int valueSize, int capacityHint, CMapCleanupValueFn cleanupFn);


/**
 * Function: CMapDispose
 * Usage:  CMapDispose(mymap);
 * ---------------------------
 * Disposes of this CMap and deallocates its memory, including the string keys
 * that CMap copied. It does *not* automatically free memory owned by pointers
 * embedded within the values. This would require knowledge of the structure of
 * the values, which the CMap does not have. It *does* loop over the values
 * calling the cleanupFn that was supplied to CMapCreate when the given CMap
 * was created.
 */

void CMapDispose(CMap *cm);


/**
 * Function: CMapCount
 * Usage:  int count = CMapCount(mymap);
 * -------------------------------------
 * Returns the number of entries currently stored in this CMap. Operates in
 * constant-time.
 */

int CMapCount(const CMap *cm);


/**
 * Function: CMapPut
 * Usage:  CMapPut(mymap, "CS107", &val);
 * --------------------------------------
 * Associates the given key with a new value in this CMap. If there is an
 * existing value for the key, it is replaced with the new value. Before being
 * overwritten, the cleanupFn that was supplied to CMapCreate is called on the
 * old value. The new value is copied from the memory pointed to by valueAddr.
 * A copy of the key string is made by the CMap to store internally. Note that
 * keys are compared case-sensitively, e.g. "binky" is not the same key as
 * "BinKy". The capacity is enlarged if necessary, an assert is raised on
 * allocation failure. Operates in constant-time (amortized).
 */

void CMapPut(CMap *cm, const char *key, const void *valueAddr);


/**
 * Function: CMapGet
 * Usage:  int val = *(int *)CMapGet(mymap, "CS107");
 * --------------------------------------------------
 * Searches this CMap for an entry with the given key and if found, returns a
 * pointer to its associated value.  If key is not found, then NULL is returned
 * as a sentinel.  Note this function returns a pointer into the CMap's
 * storage, so the pointer should be used with care. In particular, the pointer
 * returned by CMapGet can become invalid after a call that adds or removes
 * entries within the CMap.  Note that keys are compared case-sensitively,
 * e.g. "binky" is not the same key as "BinKy". Operates in constant-time.
 */

void *CMapGet(const CMap *cm, const char * key);


/**
 * Function: CMapRemove
 * Usage:  CMapRemove(mymap, "CS107");
 * -----------------------------------
 * Searches this CMap for an entry with the given key and if found, removes
 * that key and its associated value.  If key is not found, no changes are
 * made.  Before a value is removed, the cleanupFn that was supplied to
 * CMapCreate will be called on the value. The CMap frees the copy of the key
 * string it made.  Note that keys are compared case-sensitively, e.g. "binky"
 * is not the same key as "BinKy". Operates in constant-time.
 */

void CMapRemove(CMap *cm, const char * key);


/**
 * Function: CMapMap
 * Usage:  CMapMap(mymap, PrintEntry, &myData);
 * --------------------------------------------
 * Iterates over all of the entries in this CMap and calls mapfn on
 * each entry.  The callback function is called with a char * key, a pointer
 * to its associated value, and the auxData pointer. The auxData value allows
 * the client to pass extra state information if desired. If no client data is
 * needed, this argument can be NULL. The entries are processed in arbitrary
 * order. Operates in linear-time.
 */

void CMapMap(CMap *cm, CMapMapEntryFn mapfn, void *auxData);

#endif
