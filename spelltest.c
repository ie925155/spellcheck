#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "cmap.h"

static void build_hashmap(FILE *fp_corpus, CMap *map);

void printMap(const char *key, void *a, void *b)
{
	printf("%-15s %d\n", key, *(int*)a);
}

static void build_hashmap(FILE *fp_corpus, CMap *map)
{
  char ch, *ptr, token[32], reader[64] = {0x0};
	int index = 0, *value, frequency;
  while(fscanf(fp_corpus, "%s", reader) != EOF)
	{
		//if(strlen(reader) > 30) continue;
		ptr = reader;
		while((ch = *ptr++) != '\0')
			if(isalpha(ch)) token[index++] = ch;
		token[index] = '\0';
		value = CMapGet(map, token);
		frequency = (value == NULL) ? 1 : *value+1;
		(value == NULL) ? CMapPut(map, token, &frequency)
		                : CMapPut(map, token, &frequency);
		index = 0;
  }
}

int main(int argc, char *argv[])
{
  if(argc == 1 || argc == 2)
  {
    fprintf(stderr, "you have to input two arguments\n");
    return -1;
  }
  FILE *fp_corpus, *fp_spellcheck;
  char *word_check;
  fp_corpus = fopen(argv[1], "r");
  assert(fp_corpus != NULL);
  if(strchr(argv[2], '.') != NULL)
    fp_spellcheck = fopen(argv[2], "r");
  else
    word_check = argv[2];
	CMap *map = CMapCreate(sizeof(int), 1000, NULL);
  build_hashmap(fp_corpus, map);
  CMapMap(map, printMap, NULL);
  return 0;
}
