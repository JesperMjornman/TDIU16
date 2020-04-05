/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>
#include <debug.h>
#include <stdio.h>
int
main (void)
{
	int i = 0;
  halt ();
	exit(1000);
  /* not reached */
}
