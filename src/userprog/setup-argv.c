#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "setup-argv.h"


bool exists_in(char c, const char* d)
{
  int i = 0;
  while (d[i] != '\0' && d[i] != c)
    ++i;
  return (d[i] == c);
}

/* Return the number of words in 'buf'. A word is defined as a
 * sequence of characters not containing any of the characters in
 * 'delimeters'.
 * NOTE: arguments must be '\0'-terminated c-strings
 */
int count_args(const char* buf, const char* delimeters)
{
  int i = 0;
  int prev_was_delim;
  int cur_is_delim = true;
  int argc = 0;

  while (buf[i] != '\0')
  {
    prev_was_delim = cur_is_delim;
    cur_is_delim = exists_in(buf[i], delimeters);
    argc += (prev_was_delim && !cur_is_delim);
    ++i;
  }
  return argc;
}

void* setup_main_stack(const char* command_line, void* stack_top)
{
  /* Variable "esp" stores an address, and at the memory loaction
   * pointed out by that address a "struct main_args" is found.
   * That is: "esp" is a pointer to "struct main_args" */
  struct main_args* esp;
  int argc;
  int total_size;
  int line_size;
  //int cmdl_size;

  /* "cmd_line_on_stack" and "ptr_save" are variables that each store
   * one address, and at that address (the first) char (of a possible
   * sequence) can be found. */
  char* cmd_line_on_stack;
  char* ptr_save;
  int i = 0;


  /* calculate the bytes needed to store the command_line */
  line_size = strlen(command_line);
  //STACK_DEBUG("# line_size = %d\n", line_size);

  /* round up to make it even divisible by 4 */
  line_size += (!(line_size % 4) ? 0 : 4 - (line_size % 4)); 
  //STACK_DEBUG("# line_size (aligned) = %d\n", line_size);

  /* calculate how many words the command_line contain */
  argc = count_args(command_line, " "); // Space is delimiter, think "./a.out this is args" <- has 3(4) arguments.
  //STACK_DEBUG("# argc = %d\n", argc);

  /*
	   calculate the size needed on our simulated stack
	   line_size: command line mem.
		 argc * 4 : number of arguments contains pointers to cmd_line_on_stack, each 4 bytes.
		 16       : argv[0] + argv + argc (address containing its int) + esp (return address)
		 Look at 07_main_stack.pdf for clearer description.
	*/
  total_size = line_size + (argc * 4) + 16;
  //STACK_DEBUG("# total_size = %d\n", total_size);


  /* calculate where the final stack top will be located */
  esp = (struct main_args*)((unsigned*)stack_top - total_size); // Unsigned int to avoid possible problems with addressing (instead of unsigned short).

  /* setup return address and argument count */
  esp->ret = NULL;  // Should be NULL according to pintos wiki.
  esp->argc = argc; // Already calculated, argc no pointer so just assign value.
  /* calculate where in the memory the argv array starts */
  esp->argv = (char**)(esp + 1); // Cast address to correct type, char**

	/* calculate where in the memory the words is stored */
	cmd_line_on_stack = ((char*)stack_top - line_size); // Cast to char* as it's what type cmd_line_on_stack is.

	/* copy the command_line to where it should be in the stack */
	strlcpy(cmd_line_on_stack, command_line, line_size);

	/* build argv array and insert null-characters after each word */
	/* See string.c for more information. */
	for (char *token = strtok_r(cmd_line_on_stack, " ", &ptr_save); token != NULL; token = strtok_r(NULL, " ", &ptr_save))
	{
		debug ("arg(%d) = \"%s\"\n", i, token);
		esp->argv[i++] = token;
	}

	return esp; /* the new stack top */
}
