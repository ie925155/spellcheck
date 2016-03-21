#include "cmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct CMapImplementation{
  void *buckets;
  int valueSize;
  int numBuckets;
  int elemCount;
  CMapCleanupValueFn cleanupFn;
};

typedef struct _Cell_ {
  struct Cell *next;
} Cell;

/* use djb2 algorithm */
static int hashCode(const char *str)
{
  unsigned long hash = 5381;
  int c;
  while(c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash *33 + c */
  return (hash & 0xFFFFFFFF);
}

CMap *CMapCreate(int valueSize, int capacityHint, CMapCleanupValueFn cleanupFn)
{
  CMap *cm = (CMap*) malloc(sizeof(struct CMapImplementation));
  assert(cm != NULL);
  cm->buckets = malloc(sizeof(Cell)*capacityHint);
  memset(cm->buckets, 0x00, sizeof(Cell)*capacityHint);
  cm->valueSize = valueSize;
  cm->numBuckets = capacityHint;
  cm->elemCount = 0;
  cm->cleanupFn = cleanupFn;
  assert(cm->buckets != NULL);
  return cm;
}

void CMapDispose(CMap *cm)
{}

int CMapCount(const CMap *cm)
{
  return cm->elemCount;
}

void CMapPut(CMap *cm, const char *key, const void *valueAddr)
{
  int index = hashCode(key) % cm->numBuckets;
  void *blob = malloc(sizeof(Cell) + (strlen(key)+1) + cm->valueSize);
  assert(blob != NULL);
  Cell *head = (Cell*)(char*)cm->buckets + (index * sizeof(Cell));
  while(head->next != NULL){
    head = head->next;
    if(strcmp(key, (char*)head + sizeof(Cell)) == 0){
      void *value = (char*)head+sizeof(Cell)+strlen((char*)head+sizeof(Cell))+1;
      cm->cleanupFn(value);
      memcpy(value, valueAddr, cm->elemSize);
      return;
    }
  }
  head->next = blob;
  memcpy((char*)blob + sizeof(Cell), key, strlen(key)+1);
  memcpy((char*)blob + sizeof(Cell) + strlen(key)+1, valueAddr, cm->valueSize);
  cm->elemCount++;
}

void *CMapGet(const CMap *cm, const char * key)
{return NULL;}

void CMapRemove(CMap *cm, const char * key)
{};

void CMapMap(CMap *cm, CMapMapEntryFn mapfn, void *auxData)
{};
