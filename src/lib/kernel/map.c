#include "lib/kernel/map.h"
#include <stdlib.h>
#include <stdio.h>
#include "threads/malloc.h"

void map_init(struct map* m)
{
	list_init(&m->content);
	lock_init(&m->mlock);
	m->next_key = 2;
}

key_t map_insert(struct map* m, value_t v)
{
	lock_acquire(&m->mlock);

	if(map_find(m, m->next_key) != NULL)
	{
		lock_release(&m->mlock);
		return -1; // Error in insertion.
	}

	struct association *new_elem = (struct association*)malloc(sizeof(struct association));
	if(new_elem == NULL)
	{
		lock_release(&m->mlock);
		return -1;
	}

	new_elem->key = m->next_key++;
	new_elem->value = v;

	list_push_back(&m->content, &new_elem->elem);

	lock_release(&m->mlock);
	return new_elem->key;
}

int map_insert_from_key(struct map *m, value_t v, key_t k)
{
	lock_acquire(&m->mlock);

	if (map_find(m, k) != NULL)
	{
		lock_release(&m->mlock);
		return -1;
	}

	struct association *new_elem = malloc(sizeof(struct association));
	if(new_elem == NULL)
	{
		lock_release(&m->mlock);
		return -1;
	}

	new_elem->key = k;
	new_elem->value = v;

	list_push_back(&m->content, &new_elem->elem);
	lock_release(&m->mlock);
	return k;
}

value_t map_find(struct map* m, key_t k)
{
	struct association *e = map_find_associative(m, k);
	return e == NULL ? NULL : e->value;
}

value_t map_remove(struct map* m, key_t k)
{
	lock_acquire(&m->mlock);
	struct association *e = map_find_associative(m, k);
	if(e == NULL || k < 0)
	{
		lock_release(&m->mlock);
		return NULL;
	}
	value_t tmp = e->value;
	list_remove(&e->elem);
	free(e);
	lock_release(&m->mlock);
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
	lock_acquire(&m->mlock);
	for (struct list_elem *it = list_begin(&m->content); it != list_end(&m->content); it = list_next(it))
	{
		struct association *e = list_entry(it, struct association, elem);
		if (cond(e->key, e->value, aux))
		{
			it = map_remove_from_pointer(e);
			it = list_prev(it);
		}
	}
	lock_release(&m->mlock);
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

value_t map_get_from_pointer(struct list_elem *it)
{
	if(it != NULL)
	{
		struct association *e = list_entry(it, struct association, elem);
		return(e->value);
	}
	return NULL;
}

struct list_elem *map_remove_from_pointer(struct association *it)
{
	struct list_elem *tmp = list_remove(&it->elem);
	free(it);
	return tmp;
}

size_t free_all_mem(struct map *m)
{
//	lock_acquire(&m->lock);
	struct list_elem *it = list_begin(&m->content);
	while(it != list_end(&m->content))
	{
		struct association *e = list_entry(it, struct association, elem);
		it = map_remove_from_pointer(e);
	}
//	lock_release(&m->lock);
	return(!list_size(&m->content));
}
