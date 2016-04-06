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

CVector *g_leader_board;
CELL_INFO g_ed_info;
_Bool g_is_stop = false;
static int g_current_count = 0;

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

static void update_leader_board(const int count, CELL_INFO *info,
	const char *key, void *a, void *b)
{
	int ed = edit_distance(key, strlen(key), b, strlen(b));
	g_ed_info.key = key;
	g_ed_info.edit_distance = ed;
	g_ed_info.frequency = *(int*)a;
	//printf("%-30s %d \tedit_distance=%d\n", key, *(int*)a, ed);
	if(count < MAX_CORRECTION)
	{
		int found = CVectorSearch(g_leader_board, key, compareFn, 0, false);
		if(found == -1) CVectorAppend(g_leader_board, &g_ed_info);
	}
	else
	{
		for(int i = 0 ; i < count ; i++)
		{
		  info = (CELL_INFO*)CVectorNth(g_leader_board, i);
			if(ed < info->edit_distance)
			{
			  CVectorReplace(g_leader_board, &g_ed_info, i);
			  break;
		  }
			else if(ed == info->edit_distance && g_ed_info.frequency > info->frequency)
			{
			  CVectorReplace(g_leader_board, &g_ed_info, i);
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
	int count = CVectorCount(g_leader_board);
	if(count == MAX_CORRECTION)
	{
		int well_choice = 0;
		for(int i = 0 ; i < count ; i++)
		{
		  info = (CELL_INFO*)CVectorNth(g_leader_board, i);
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
	CMap *map_corpus = (CMap*)b;
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

int main(int argc, char *argv[])
{
  if(argc == 1 || argc == 2)
  {
    fprintf(stderr, "you have to input two arguments\n");
    return -1;
  }
  FILE *fp_corpus, *fp_document = NULL;
  char *word_check;
  fp_corpus = fopen(argv[1], "r");
  assert(fp_corpus != NULL);
  if(strchr(argv[2], '.') != NULL)
    fp_document = fopen(argv[2], "r");
  else
    word_check = argv[2];
	CMap *map_corpus = CMapCreate(sizeof(int), 1000, NULL);
	g_leader_board = CVectorCreate(sizeof(CELL_INFO), MAX_CORRECTION, NULL);
	//build model
	printf("build the model...\n");
  build_hashmap(fp_corpus, map_corpus);
	printf("find the best 5 corrections...\n");
	if(fp_document == NULL)
	{
		int *val = (int *)CMapGet(map_corpus, word_check);
		if(val == NULL)
		{
  	  CMapMap(map_corpus, find_possible_spelling, word_check);
			CVectorSort(g_leader_board, vecCompareFn);
			//print out the answer
			printf("%s :", word_check);
			int count = CVectorCount(g_leader_board);
			CELL_INFO *info;
			for(int i = 0 ; i < count ; i++){
				info = (CELL_INFO*)CVectorNth(g_leader_board, i);
				printf("%s ", info->key);
			}
			printf("\n");
			g_is_stop = false;
			g_current_count = 0;
		}else
			printf("the word %s is correct\n", word_check);
	}
	else
	{
		CMap *map_document = CMapCreate(sizeof(int), 1000, NULL);
		build_hashmap(fp_document, map_document);
  	CMapMap(map_document, find_misspelling, map_corpus);
		CMapDispose(map_document);
	}
	//CVectorMap(g_leader_board, vectorMapFn, NULL);
	CVectorDispose(g_leader_board);
	CMapDispose(map_corpus);
  return 0;
}
