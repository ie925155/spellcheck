#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "cmap.h"
#include "cvector.h"

#define MAX_TOKEN_SIZE 31
#define MAX_CORRECTION 5

typedef struct {
	const char *key;
	int edit_distance;
	int frequency;
} CELL_INFO;

CELL_INFO g_ed_info;
_Bool g_is_stop = false;
static int g_current_count = 0;

typedef struct {
	char *word_check;
	CVector *leader_board;
} CORRECTION_INFO;

typedef struct {
	CMap *map_corpus;
	CVector *misspelling;
	char *keepPtr[100];
} CORPUS_INFO;

int g_count=0;

/* Function Prototypes */
static void build_hashmap(FILE *fp_corpus, CMap *map);
static int edit_distance(const char *s, int len_s, const char *t, int len_t);
static void update_leader_board(const int count, CELL_INFO *info,
	const char *key, void *a, void *b);

void vectorMapFn(void *elemAddr, void *auxData)
{
	CELL_INFO *info = (CELL_INFO*)elemAddr;
	printf("info key=%-20s ed=%d frequency=%d\n", info->key, info->edit_distance,
	  info->frequency);
}

int vecCompareFn(const void *elemAddr1, const void *elemAddr2)
{
  return ((CELL_INFO*)elemAddr1)->edit_distance >
		((CELL_INFO*)elemAddr2)->edit_distance;
}

int compareFn(const void *elemAddr1, const void *elemAddr2)
{
	return strcmp(elemAddr1, ((CELL_INFO*)elemAddr2)->key);
}

inline static int min(int a, int b, int c)
{
	int temp = a;
	if(temp > b) temp = b;
	if(temp > c) return c;
	return temp;
}

/* resursive version */
static int edit_distance(const char *s, int len_s, const char *t, int len_t)
{
	int cost;
	if(len_s == 0) return len_t;
	if(len_t == 0) return len_s;
	cost = ((s[len_s-1] == t[len_t-1])) ? 0 : 1;
	return min(edit_distance(s, len_s-1, t, len_t)+1,				//delete
						 edit_distance(s, len_s, t, len_t-1)+1,				//insert
						 edit_distance(s, len_s-1, t, len_t-1)+cost);	//same or substitute
}

static void update_leader_board(const int count, CELL_INFO *info,
	const char *key, void *a, void *b)
{
	char *word_check = ((CORRECTION_INFO*)b)->word_check;
	CVector *leader_board = ((CORRECTION_INFO*)b)->leader_board;
	int ed = edit_distance(key, strlen(key), word_check,
	  strlen(word_check));
	g_ed_info.key = key;
	g_ed_info.edit_distance = ed;
	g_ed_info.frequency = *(int*)a;
	//printf("%-30s %d \tedit_distance=%d\n", key, *(int*)a, ed);
	if(count < MAX_CORRECTION)
	{
		int found = CVectorSearch(leader_board, key, compareFn, 0, false);
		if(found == -1) CVectorAppend(leader_board, &g_ed_info);
	}
	else
	{
		for(int i = 0 ; i < count ; i++)
		{
		  info = (CELL_INFO*)CVectorNth(leader_board, i);
			if(ed < info->edit_distance)
			{
			  CVectorReplace(leader_board, &g_ed_info, i);
			  break;
		  }
			else if(ed == info->edit_distance && g_ed_info.frequency > info->frequency)
			{
			  CVectorReplace(leader_board, &g_ed_info, i);
			  break;
			}
		}
	}
}

void find_possible_spelling(const char *key, void *a, void *b)
{
	// prune the work, find the 5 best correction ed <=3 then skip the rest
	if(g_is_stop) return;
	g_current_count++;
	CELL_INFO *info;
	int count = CVectorCount(((CORRECTION_INFO*)b)->leader_board);
	if(count == MAX_CORRECTION)
	{
		int well_choice = 0;
		for(int i = 0 ; i < count ; i++)
		{
		  info = (CELL_INFO*)CVectorNth(((CORRECTION_INFO*)b)->leader_board, i);
			if(info->edit_distance <= 3) well_choice++;
		}
		if(well_choice == count && g_current_count > 200)
		{
			 g_is_stop = true;
		   return;
	  }
	}
	update_leader_board(count, info, key, a, b);
}

void find_misspelling(const char *key, void *a, void *b)
{
	CMap *map_corpus = ((CORPUS_INFO*)b)->map_corpus;
	if(CMapGet(map_corpus, key) == NULL)
	{
		((CORPUS_INFO*)b)->keepPtr[g_count++] = (char*)key;
	  CVectorAppend(((CORPUS_INFO*)b)->misspelling,
		  &((CORPUS_INFO*)b)->keepPtr[g_count-1]);
	}
}

static void build_hashmap(FILE *fp_corpus, CMap *map)
{
  char ch, *ptr, token[MAX_TOKEN_SIZE], reader[MAX_TOKEN_SIZE] = {0x0};
	int index, *value, frequency;
	_Bool is_discard;
  while(fscanf(fp_corpus, "%30s", reader) != EOF)
	{
		index = is_discard = 0;
		ptr = reader;
		while((ch = *ptr++) != '\0')
		{
			if(isalpha(ch))
			  token[index++] = tolower(ch);  //case-insensitivity
			else{
				is_discard = 1;
				break;
			}
		}
		if(is_discard) continue;
		token[index] = '\0';
		value = CMapGet(map, token);
		frequency = (value == NULL) ? 1 : *value+1;
		(value == NULL) ? CMapPut(map, token, &frequency)
		                : CMapPut(map, token, &frequency);
  }
}


static void find_best_corrections(CORPUS_INFO *corpus_info, CORRECTION_INFO *correction)
{
		correction->leader_board = CVectorCreate(sizeof(CELL_INFO), MAX_CORRECTION, NULL);
	  CMapMap(corpus_info->map_corpus, find_possible_spelling, correction);
		CVectorSort(correction->leader_board, vecCompareFn);
		//print out the answer
		printf("%s :", correction->word_check);
		int count = CVectorCount(correction->leader_board);
		CELL_INFO *info;
		for(int i = 0 ; i < count ; i++){
			info = (CELL_INFO*)CVectorNth(correction->leader_board, i);
			printf("%s ", info->key);
		}
		printf("\n");
		g_is_stop = false;
		g_current_count = 0;
		CVectorDispose(correction->leader_board);
}

int main(int argc, char *argv[])
{
  if(argc == 1 || argc == 2)
  {
    fprintf(stderr, "you have to input two arguments\n");
    return -1;
  }
  FILE *fp_corpus, *fp_document = NULL;
	CORRECTION_INFO correction;
	CORPUS_INFO corpus_info;
  fp_corpus = fopen(argv[1], "r");
  assert(fp_corpus != NULL);
  if(strchr(argv[2], '.') != NULL)
    fp_document = fopen(argv[2], "r");
  else
    correction.word_check = argv[2];
	corpus_info.map_corpus = CMapCreate(sizeof(int), 1000, NULL);
	//build model
	printf("build the model...\n");
  build_hashmap(fp_corpus, corpus_info.map_corpus);
	printf("find the best 5 corrections...\n");
	if(fp_document == NULL)
	{
		int *val = (int *)CMapGet(corpus_info.map_corpus, correction.word_check);
		if(val == NULL)
			find_best_corrections(&corpus_info, &correction);
		else
			printf("the word %s is correct\n", correction.word_check);
	}
	else
	{
		CMap *map_document = CMapCreate(sizeof(int), 1000, NULL);
		corpus_info.misspelling = CVectorCreate(sizeof(char*), 100, NULL);
		build_hashmap(fp_document, map_document);
  	CMapMap(map_document, find_misspelling, &corpus_info);
		int misspelling_count = CVectorCount(corpus_info.misspelling);
		for(int i = 0 ; i < misspelling_count ; i++){
			correction.word_check = *(char**)CVectorNth(corpus_info.misspelling, i);
			find_best_corrections(&corpus_info, &correction);
		}
		CVectorDispose(corpus_info.misspelling);
		CMapDispose(map_document);
	}
	CMapDispose(corpus_info.map_corpus);
	if(fp_document != NULL) fclose(fp_document);
	fclose(fp_corpus);
  return 0;
}
