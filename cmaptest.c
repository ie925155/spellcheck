#include "cmap.h"
#include "bool.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

void mapScalar(const char *key, void *a, void *b) {
	int x = *(int*)a;
	int y = *(int*)b;
	printf("%s %d\n", key, x * y);
}

void printMap(const char *key, void *a, void *b)
{
		printf("%s %d\n", key, *(int*)a);
}

static void simpleTest()
{
	CMap *map = CMapCreate(sizeof(int), 10, NULL);

	char *colors[] = {"red", "blue", "green", "yellow", "pink",
		"Acajou", "Acid Green", "Aero", "Aero Blue", "Absolute Zero",
	 "Alien Armpit", "Alizarin Crimson", "Alloy Orange", "Almond" , "Amaranth Purple",
	 "fruit_weights", "Apples", "Oranges", "Grapes", "Hashmap_compare"};

	for(int i = 0; i < 20; i++) {
		CMapPut(map, colors[i], &i);
	}

	for(int i = 0; i < 20; i++) {
		int *value = CMapGet(map, colors[i]);
		printf("value=%d\n", *value);
	}

	int scalar = 2;

	CMapRemove(map, "red");
	CMapRemove(map, "yellow");
	CMapRemove(map, "Acajou");
	CMapRemove(map, "pink");
	CMapRemove(map, "green");
	CMapRemove(map, "Apples");
	CMapRemove(map, "Oranges");
	CMapRemove(map, "blue");
	CMapMap(map, mapScalar, &scalar);
	CMapDispose(map);
}

static void BuildTableOfLetterCounts(CMap *counts)
{
  //struct frequency localFreq, *found;
	char buffer[255];
	int max_count=0;
  int ch;
  //FILE *fp = fopen("cmaptest.c", "r"); // open self as file
  //FILE *fp = fopen("/media/sheldon/DATA/Dropbox/cs107/Programming_Assignments/assn-3-vector-hashset-data/thesaurus.txt", "r"); // open self as file
  FILE *fp = fopen("/mnt/hgfs/glay_luncy/Dropbox/cs107/Programming_Assignments/assn-3-vector-hashset-data/thesaurus.txt", "r"); // open self as file
  assert(fp != NULL);
	int record=0;
	bool recordFlag = false;
  while ((ch = getc(fp)) != EOF) {
		recordFlag = true;
		if(ch == ','){
		  buffer[record] = '\0';
			int *frequency = CMapGet(counts, buffer);
			if(frequency == NULL){
			  int temp = 1;
				CMapPut(counts, buffer, &temp);  // enter should overwrite if needed
			}else{
				int temp = *frequency+1;  // increment if already there
				CMapPut(counts, buffer, &temp);
			}
			record=0;
			recordFlag = false;
			memset(buffer, 0x00, 255);
			max_count++;
			if(max_count == 5000) break;
		}
		if(recordFlag)
		  buffer[record++] = tolower(ch);
  }
	CMapMap(counts, printMap, NULL);
  fclose(fp);
}

int main() {
	simpleTest();
	CMap *map = CMapCreate(sizeof(int), 180000, NULL);
	BuildTableOfLetterCounts(map);
	printf("total %d elements\n", CMapCount(map));
	CMapDispose(map);
}
