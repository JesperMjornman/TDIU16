#include <stdio.h>
#include "list.h"
#include "dictionary.h"
#include <assert.h>

int main(int argc, char *argv[])
{
	struct map dict;
	map_init(&dict);

	printf("Added elem with key: %d\n", map_insert(&dict, "10"));
	printf("Added elem with key: %d\n", map_insert(&dict, "12"));
	
	assert(list_size(&dict) == 2);

	assert(map_find(&dict, 0) == "10");
	assert(map_find(&dict, 1) == "12");

	return 0;
}

