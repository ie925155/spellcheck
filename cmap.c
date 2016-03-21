#include "cmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct _Cell_ {
  struct Cell *next;
} Cell;

typedef struct _bucket_{
   Cell *next;
} Bucket;

struct CMapImplementation{
  Bucket *buckets;
  int valueSize;
  int numBuckets;
  int elemCount;
  CMapCleanupValueFn cleanupFn;
};

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
  cm->buckets = (Bucket*)malloc(sizeof(Bucket)*capacityHint);
  int i;
  for(i = 0 ; i < capacityHint ; i++){
    Bucket bucket = {0x00};
    memcpy(cm->buckets + i, &bucket, sizeof(Bucket));
  }
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
  Bucket *head = cm->buckets + index;
  Cell *cell = head->next;
  while(cell->next != NULL){
    if(strcmp(key, (char*)cell + sizeof(Cell)) == 0){
      void *value = (char*)cell+sizeof(Cell)+strlen((char*)cell+sizeof(Cell))+1;
      cm->cleanupFn(value);
      memcpy(value, valueAddr, cm->valueSize);
      return;
    }
  }
  cell->next = blob;
  memset(blob, 0x00, sizeof(Cell));
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
