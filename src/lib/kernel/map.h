#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <stdbool.h>
<<<<<<< HEAD
#include "list.h"

#define PANIC(message) do { printf("PANIC: %s\n", message); exit(1); } while (0)

typedef char* value_t;
=======
#include "lib/kernel/list.h"

/*
 * Remember to type cast to correct value in case of printing.
 */
typedef void* value_t;
>>>>>>> dev
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
	int next_key; // Konstig?
};

void map_init(struct map* m);

key_t map_insert(struct map* m, value_t v);
<<<<<<< HEAD
=======
bool map_insert_from_key(struct map *m, value_t v, key_t k);
>>>>>>> dev
value_t map_find(struct map* m, key_t k);
value_t map_remove(struct map* m, key_t k);

void map_for_each(struct map* m,
	void(*exec)(key_t k, value_t v, int aux),
	int aux);

void map_remove_if(struct map* m,
	bool(*cond)(key_t k, value_t v, int aux),
	int aux);

size_t free_all_mem(struct map *dict);
struct association *map_find_associative(struct map *m, key_t k);
<<<<<<< HEAD
struct list_elem *map_remove_from_pointer(struct map *m, struct association *it);
=======
struct list_elem *map_remove_from_pointer(struct association *it);
>>>>>>> dev
#endif
