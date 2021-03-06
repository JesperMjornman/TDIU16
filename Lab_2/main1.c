#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "list.h"
#include "map.h"
#include <assert.h>
void printKeyPair(key_t k, value_t v, int aux);
bool isEven(key_t k, value_t v, int aux);
int main(int argc, char *argv[])
{
	struct map dict;
	map_init(&dict);

	printf("Added elem with key: %d\n", map_insert(&dict, "10"));
	printf("Added elem with key: %d\n", map_insert(&dict, "12"));

	assert(list_size(&dict.content) == 2);
	assert(map_find(&dict, 0) == "10");
	assert(map_find(&dict, 1) == "12");

	map_remove(&dict, 0);

	assert(list_size(&dict.content) == 1);
	assert(map_find(&dict, 0) == NULL);
	assert(map_find(&dict, 1) == "12");

	printf("Added elem with key: %d\n", map_insert(&dict, "13"));
	printf("Added elem with key: %d\n", map_insert(&dict, "14"));
	printf("Added elem with key: %d\n", map_insert(&dict, "16"));

	printf("All elements printed with aux = 0: \n");
	map_for_each(&dict, &printKeyPair, 0);
	printf("All elements printed with aux = 1: \n");
	map_for_each(&dict, &printKeyPair, 1);

	map_remove_if(&dict, &isEven, 0);
	printf("After removing all even numbers: \n");
	map_for_each(&dict, &printKeyPair, 0);

	printf("Release all mem.\n");
	assert(free_all_mem(&dict));
	return 0;
}

void printKeyPair(key_t k, value_t v, int aux)
{
	if(!aux)
		printf("%d : %s\n", k, v);
	else
		printf("Key [%d] to Value [%s]\n", k, v);
}

bool isEven(key_t k, value_t v, int aux)
{
	return(!(atoi(v) % 2));
}
