#include "dictionary.h"
#include <stdio.h>
void map_init(struct map* m)
{
	list_init(&m->content);
	m->next_key = 0;
}

key_t map_insert(struct map* m, value_t v)
{
	struct association *new_elem = malloc(sizeof(struct association));
	new_elem->key = m->next_key++;
	new_elem->value = v;

	list_push_front(&m->content, &new_elem->elem);
	return new_elem->key;
}

value_t map_find(struct map* m, key_t k)
{	
	struct association *e = map_find_associative(m, k);
	return e->value;
}

value_t map_remove(struct map* m, key_t k)
{
	struct association *e = map_find_associative(m, k);
	list_remove(&e->elem);
	free(e);
}

void map_for_each(struct map* m,
	void(*exec)(key_t k, value_t v, int aux),
	int aux)
{

}

void map_remove_if(struct map* m,
	bool(*cond)(key_t k, value_t v, int aux),
	int aux)
{

}

struct association *map_find_associative(struct map *m, key_t k)
{
	for (struct list_elem *it = list_begin(&m->content); it != list_end(&m->content); it = list_next(it))
	{
		struct association *e = list_entry(it, struct association, elem);
		if (e->key == k)
			return e;
	}
	return NULL;
}
