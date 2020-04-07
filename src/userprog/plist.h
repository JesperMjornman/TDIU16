#ifndef _PLIST_H_
#define _PLIST_H_
#include <stdio.h>
#include <stdbool.h>
#include "lib/kernel/list.h"
#include "lib/kernel/map.h"

/* Place functions to handle a running process here (process list).

   plist.h : Your function declarations and documentation.
   plist.c : Your implementation.

   The following is strongly recommended:

   - A function that given process inforamtion (up to you to create)
     inserts this in a list of running processes and return an integer
     that can be used to find the information later on.

   - A function that given an integer (obtained from above function)
     FIND the process information in the list. Should return some
     failure code if no process matching the integer is in the list.
     Or, optionally, several functions to access any information of a
     particular process that you currently need.

   - A function that given an integer REMOVE the process information
     from the list. Should only remove the information when no process
     or thread need it anymore, but must guarantee it is always
     removed EVENTUALLY.

   - A function that print the entire content of the list in a nice,
     clean, readable format.

 */

struct processInfo
{
	int pid;
	int parent_pid;
	int exit_status;
	bool alive;
	bool parent_alive;
};

/*
 	Uses map.h for functionality
	Some different solutions to the usage value_t is void*
*/
void plist_init(struct map *pl);
void plist_print(struct map *pl);
void plist_print_format(key_t k UNUSED, value_t v, int aux UNUSED);
bool plist_to_be_erased(key_t k UNUSED, value_t v, int aux UNUSED);

int plist_insert(struct map *pl, value_t *v, key_t k);

value_t plist_find(struct map* pl, key_t k);
value_t plist_remove(struct map* pl, key_t k);

size_t plist_free_all_mem(struct map *pl);
struct processInfo *plist_create_process(int pid, int parent_pid);
void plist_remove_if(struct map* m,
	bool(*cond)(key_t k, value_t v, int aux),
	int aux);
#endif
