#include <stddef.h>
#include "userprog/plist.h"
void plist_init(struct map *pl)
{
	map_init(pl);
}

void plist_print_format(key_t k UNUSED, value_t v, int aux UNUSED)
{
	struct processInfo *p = v;
	printf("%6d  %8d  %14d %9d %11d \n",
					p->pid,
					p->parent_pid,
					p->exit_status,
					p->alive,
					p->parent_alive);
}

void plist_print(struct map *pl)
{
	printf("|  PID  |  P-PID  |  EXIT-STATUS  |  ALIVE  |  P-ALIVE  |\n");
	map_for_each(pl, &plist_print_format, 0);
}

int plist_insert(struct map *m, value_t *v, key_t k)
{
	return map_insert_from_key(m, v, k);
}

value_t plist_find(struct map* m, key_t k)
{
	return map_find(m, k);
}
value_t plist_remove(struct map* m, key_t k)
{
	return map_remove(m, k);
}

size_t plist_free_all_mem(struct map *map)
{
	return free_all_mem(map);
}
