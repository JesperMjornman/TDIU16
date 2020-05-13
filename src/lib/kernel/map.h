#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <stdbool.h>
#include "lib/kernel/list.h"
#include "threads/synch.h"
/*
 * Remember to type cast to correct value in case of printing.
 */
typedef void* value_t;
typedef int key_t;

struct association
{
	key_t key;
	value_t value;
	struct list_elem elem;
};

struct map
{
	struct list content;
	struct lock mlock;
	int next_key;
};

void map_init(struct map* m);

/**
	 Use with caution, incrementing next_key might interfere with other functions.
	 Is used for file mapping, otherwise set the keys (use map_insert_from_key).

	 Inserts and returns key, in the case of file mapping returns fd.
	 Using map_insert starts from key = 2 and then increments for each element.
 */
key_t map_insert(struct map* m, value_t v);

/**
	Insert by defining key, returns the key specified if
	insertio successful. 
*/
key_t map_insert_from_key(struct map *m, value_t v, key_t k);

value_t map_find	(struct map* m, key_t k);
value_t map_remove(struct map* m, key_t k);
value_t map_get_from_pointer(struct list_elem *it);

void map_for_each(struct map* m,
	void(*exec)(key_t k, value_t v, int aux),
	int aux);

void map_remove_if(struct map* m,
	bool(*cond)(key_t k, value_t v, int aux),
	int aux);

size_t free_all_mem(struct map *dict);
struct association *map_find_associative(struct map *m, key_t k);
struct list_elem *map_remove_from_pointer(struct association *it);
#endif
