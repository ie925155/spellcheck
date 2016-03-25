#include "cmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct Cell {
  struct Cell *next;
};

typedef struct {
   struct Cell *next;
} Bucket;

struct CMapImplementation{
  Bucket *buckets;
  int valueSize;
  int numBuckets;
  int elemCount;
  CMapCleanupValueFn cleanupFn;
};

/* use djb2 algorithm */
static unsigned int hashCode(const char *str)
{
  unsigned long hash = 5381;
  int c;
  while((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return (hash & 0xFFFFFFFF);
}

CMap *CMapCreate(int valueSize, int capacityHint, CMapCleanupValueFn cleanupFn)
{
  CMap *cm = (CMap*) malloc(sizeof(struct CMapImplementation));
  assert(cm != NULL);
  cm->buckets = (Bucket*)malloc(sizeof(Bucket)*capacityHint);
  for(int i = 0 ; i < capacityHint ; i++){
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
{
  for(int i = 0 ; i < cm->numBuckets ; i++){
    Bucket *bucket = cm->buckets + i;
    struct Cell *cell = bucket->next;
    while(cell != NULL){
      void *value = (char*)cell+sizeof(struct Cell)+strlen((char*)cell+sizeof(struct Cell))+1;
      if(cm->cleanupFn != NULL)
        cm->cleanupFn(value);
      struct Cell *temp = cell;
      cell = cell->next;
      free(temp);
    }
  }
  free(cm->buckets);
}

int CMapCount(const CMap *cm)
{
  return cm->elemCount;
}

void CMapPut(CMap *cm, const char *key, const void *valueAddr)
{
  int index = hashCode(key) % cm->numBuckets;
  //printf("index=%d key=%s\n", index, key);
  void *blob = malloc(sizeof(struct Cell) + (strlen(key)+1) + cm->valueSize);
  assert(blob != NULL);
  Bucket *bucket = cm->buckets + index;
  struct Cell *cell = bucket->next;
  while(cell != NULL){
    if(strcmp(key, (char*)cell + sizeof(struct Cell)) == 0){
      void *value = (char*)cell+sizeof(struct Cell)+strlen((char*)cell+sizeof(struct Cell))+1;
      if(cm->cleanupFn != NULL)
        cm->cleanupFn(value);
      memcpy(value, valueAddr, cm->valueSize);
      return;
    }
    if(cell->next == NULL){
      cell->next = blob;
      memset(blob, 0x00, sizeof(struct Cell));
      memcpy((char*)blob + sizeof(struct Cell), key, strlen(key)+1);
      memcpy((char*)blob + sizeof(struct Cell) + strlen(key)+1, valueAddr, cm->valueSize);
      cm->elemCount++;
      return;
    }
    cell = cell->next;
  }
  bucket->next = blob;
  memset(blob, 0x00, sizeof(struct Cell));
  memcpy((char*)blob + sizeof(struct Cell), key, strlen(key)+1);
  memcpy((char*)blob + sizeof(struct Cell) + strlen(key)+1, valueAddr, cm->valueSize);
  cm->elemCount++;
}

void *CMapGet(const CMap *cm, const char * key)
{
  for(int i = 0 ; i < cm->numBuckets ; i++){
    Bucket *bucket = cm->buckets + i;
    struct Cell *cell = bucket->next;
    while(cell != NULL){
      if(strcmp(key, (char*)cell + sizeof(struct Cell)) == 0){
          return (char*)cell + sizeof(struct Cell) + strlen((char*)cell + sizeof(struct Cell)) + 1;
      }
      cell = cell->next;
    }
  }
  return NULL;
}

void CMapRemove(CMap *cm, const char * key)
{
  for(int i = 0 ; i < cm->numBuckets ; i++){
    Bucket *bucket = cm->buckets + i;
    struct Cell *cell = bucket->next;
    struct Cell *prev = NULL;
    struct Cell *next = (cell == NULL) ? NULL : cell->next;
    while(cell != NULL){
      if(strcmp(key, (char*)cell + sizeof(struct Cell)) == 0){
        if(cm->cleanupFn != NULL){
          void *value = (char*)cell+sizeof(struct Cell)+strlen((char*)cell+sizeof(struct Cell))+1;
          cm->cleanupFn(value);
        }
        free(cell);
        if(prev == NULL){
          bucket->next = next;
        }else{
          prev->next = next;
        }
        cm->elemCount--;
        return;
      }
      prev = cell;
      cell = cell->next;
      if(cell != NULL) next = cell->next;
    }
  }
}

void CMapMap(CMap *cm, CMapMapEntryFn mapfn, void *auxData)
{
  for(int i = 0 ; i < cm->numBuckets ; i++){
    Bucket *bucket = cm->buckets + i;
    struct Cell *cell = bucket->next;
    while(cell != NULL){
      void *key = (char*)cell + sizeof(struct Cell);
      void *value = (char*)cell+sizeof(struct Cell)+strlen((char*)cell+sizeof(struct Cell))+1;
      mapfn(key, value, auxData);
      cell = cell->next;
    }
  }
}
