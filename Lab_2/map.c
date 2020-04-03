#include "map.h"
#include <stdlib.h>
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

	list_push_back(&m->content, &new_elem->elem);
	return new_elem->key;
}

value_t map_find(struct map* m, key_t k)
{
	struct association *e = map_find_associative(m, k);
	return e == NULL ? NULL : e->value;
}

value_t map_remove(struct map* m, key_t k)
{
	struct association *e = map_find_associative(m, k);
	if(e == NULL)
		return NULL;

	value_t tmp = e->value;
	list_remove(&e->elem);
	free(e);
	return tmp;
}

void map_for_each(struct map* m,
	void(*exec)(key_t k, value_t v, int aux),
	int aux)
{
	for (struct list_elem *it = list_begin(&m->content); it != list_end(&m->content); it = list_next(it))
	{
		struct association *e = list_entry(it, struct association, elem);
		exec(e->key, e->value, aux);
	}
}

void map_remove_if(struct map* m,
	bool(*cond)(key_t k, value_t v, int aux),
	int aux)
{
	for (struct list_elem *it = list_begin(&m->content); it != list_end(&m->content); it = list_next(it))
	{
		struct association *e = list_entry(it, struct association, elem);
		if (cond(e->key, e->value, aux))
		{
			it = map_remove_from_pointer(m, e);
			it = list_prev(it);
		}
	}
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

struct list_elem *map_remove_from_pointer(struct map *m, struct association *it)
{
	struct list_elem *tmp = list_remove(&it->elem);
	free(it);
	return tmp;
}

size_t free_all_mem(struct map *m)
{
	struct list_elem *it = list_begin(&m->content);
	while(it != list_end(&m->content))
	{
		struct association *e = list_entry(it, struct association, elem);
		it = map_remove_from_pointer(m, e);
	}
	return(!list_size(&m->content));
}
