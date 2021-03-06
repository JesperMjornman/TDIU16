#include <stddef.h>
#include "userprog/plist.h"
#include "threads/malloc.h"
#include "threads/synch.h"

static struct lock plock;

void plist_init(struct map *pl)
{
	map_init(pl);
	lock_init(&plock);
}

void plist_print_format(key_t k UNUSED, value_t v, int aux UNUSED)
{
	struct processInfo *p = v;
	debug("|%5d  |%7d  |%13d  |%7d  |%9d  |\n",
					p->pid,
					p->parent_pid,
					p->exit_status,
					p->alive,
					p->parent_alive);
}

void plist_print(struct map *pl)
{
	debug("|__PID__|__P-PID__|__EXIT-STATUS__|__ALIVE__|__P-ALIVE__|\n");
	//debug("---------------------------------------------------------\n");
	map_for_each(pl, &plist_print_format, 0);
}

int plist_insert(struct map *pl, value_t v, key_t k)
{
	return map_insert_from_key(pl, v, k);
}

value_t plist_find(struct map* pl, key_t k)
{
	return map_find(pl, k);
}

int plist_remove(struct map* pl, key_t k)
{
	struct processInfo *e = map_find_associative(pl, k)->value;
	if(e != NULL)
	{
		e->alive = false;
		plist_remove_if(pl, &plist_to_be_erased, 0);
		return 1;
	}
	return -1;
}

bool plist_to_be_erased(key_t k UNUSED, value_t v, int aux UNUSED)
{
	struct processInfo *p = v;
	return(!p->alive && !p->parent_alive);
}

size_t plist_free_all_mem(struct map *pl)
{
	lock_acquire(&plock);
	struct list_elem *it = list_begin(&pl->content);
	while(it != list_end(&pl->content))
	{
		struct association *e = list_entry(it, struct association, elem);
		struct processInfo *pi = e->value;
		it = list_remove(&e->elem);
		if(pi->parent_pid != 1)
			free(e->value);
		free(e);
	}
	lock_release(&plock);
	return(!list_size(&pl->content));
}

struct processInfo *plist_create_process(int pid, int parent_pid)
{
	struct processInfo *p = malloc(sizeof(struct processInfo));
	if(p != NULL)
	{
		p->pid = pid;
		p->parent_pid = parent_pid;
		p->exit_status = 0;
		p->alive = 1;
		p->parent_alive = (p->parent_pid == 1 ? 0 : 1);
		p->waiting = false;
		sema_init(&p->sema, 0);
  }
	return p;
}

void plist_remove_if(struct map* m,
	bool(*cond)(key_t k, value_t v, int aux),
	int aux)
{
	for (struct list_elem *it = list_begin(&m->content); it != list_end(&m->content); it = list_next(it))
	{
		struct association *e = list_entry(it, struct association, elem);
		if (cond(e->key, e->value, aux))
		{
			struct processInfo *pi = e->value;
			it = list_remove(&e->elem);
			if(pi->parent_pid != 1)
				free(e->value);
			free(e);
			it = list_prev(it);
		}
	}
}
