#include "cvector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct CVectorImplementation{
  void *elems;
  int elemSize;
  int logLength;
  int allocLength;
  CVectorCleanupElemFn cleanupFn;
};

static void vectorGrow(CVector *v)
{
  //expand to double size
  if(v->allocLength == 0)
    v->allocLength = 4;
  else
    v->allocLength *= 2;
  v->elems = realloc(v->elems, v->allocLength * v->elemSize);
}

CVector *CVectorCreate(int elemSize, int capacityHint, CVectorCleanupElemFn cleanupFn)
{
  assert(elemSize > 0);
  CVector *v = (CVector*) malloc(sizeof (struct CVectorImplementation));
  assert(v != NULL);
  v->elemSize = elemSize;
  v->logLength = 0;
  v->allocLength = capacityHint;
  v->elems = malloc(capacityHint * elemSize);
  v->cleanupFn = cleanupFn;
  assert(v->elems != NULL);
  return v;
}

void CVectorDispose(CVector *v)
{
  assert(v != NULL);
  assert(v->elems != NULL);
  if(v->cleanupFn != NULL){
    int i;
    for(i = 0 ; i < v->logLength ; i++)
      v->cleanupFn(CVectorNth(v, i));
  }
  free(v->elems);
  free(v);
}

int CVectorCount(const CVector *v)
{
  return v->logLength;
}

void *CVectorNth(const CVector *v, int index)
{
  return (char*)v->elems + (v->elemSize * index);
}

void CVectorInsert(CVector *v, const void *elemAddr, int index)
{
  assert(elemAddr != NULL);
  assert((index >= 0) && (index <= v->logLength));
  if(v->logLength == v->allocLength)
    vectorGrow(v);
  memmove((char*)v->elems + ((index+1) * v->elemSize),
    (char*)v->elems + (index * v->elemSize),
	(v->logLength - index) * v->elemSize);
  memcpy((char*)v->elems + (index * v->elemSize), elemAddr, v->elemSize);
  v->logLength++;
}

void CVectorAppend(CVector *v, const void *elemAddr)
{
  assert(elemAddr != NULL);
  if(v->logLength == v->allocLength)
    vectorGrow(v);
  memcpy((char*)v->elems + (v->logLength * v->elemSize), elemAddr, v->elemSize);
  v->logLength++;
}

void CVectorReplace(CVector *v, const void *elemAddr, int index)
{
  assert(elemAddr != NULL);
  assert(index < v->allocLength);
  memcpy((char*)v->elems + (v->elemSize * index), elemAddr, v->elemSize);
}

void CVectorRemove(CVector *v, int index)
{
  assert((index >= 0) && (index < v->logLength));
  if(v->cleanupFn != NULL)
	  v->cleanupFn((char*)v->elems + (index * v->elemSize));
  memmove((char*)v->elems + (index * v->elemSize), (char*)v->elems + ((index+1) * v->elemSize),
    (v->logLength - index) * v->elemSize);
  v->logLength--;
}

int CVectorSearch(const CVector *v, const void *key, CVectorCmpElemFn comparefn, int startIndex, bool isSorted)
{
  assert(startIndex >= 0 && startIndex <= v->logLength);
  assert(comparefn != NULL);
  int position = -1;
  if(isSorted)
  {
  	void *res = bsearch(key, (char*)v->elems + (startIndex * v->elemSize), (v->logLength - startIndex),
  	  v->elemSize, comparefn);
  	if(res != NULL)
  	{
      int i;
  		for(i = startIndex ; i < v->logLength ; i++){
  			if(res == ((char*)v->elems + (i * v->elemSize))){
  				position = i;
  			  break;
  			}
  		}
  	}
  }
  else
  {
    int i;
  	for(i = startIndex ; i < v->logLength ; i++){
  		if(memcmp(key, (char*)v->elems + (i * v->elemSize), v->elemSize) == 0){
  			position = i;
  			break;
  		}
  	}
  }
  return position;
}

void CVectorSort(CVector *v, CVectorCmpElemFn comparefn)
{
  qsort(v->elems, v->logLength, v->elemSize, comparefn);
}

void CVectorMap(CVector *v, CVectorMapElemFn mapfn, void *auxData)
{
  assert(mapfn != NULL);
  int i;
  for(i = 0 ; i < v->logLength ; i++){
	  mapfn((char*)v->elems + (i * v->elemSize), auxData);
  }
}
