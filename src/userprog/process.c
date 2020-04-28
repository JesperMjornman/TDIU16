#include <debug.h>
#include <stdio.h>
#include <string.h>

#include "userprog/gdt.h"      /* SEL_* constants */
#include "userprog/process.h"
#include "userprog/load.h"
#include "userprog/pagedir.h"  /* pagedir_activate etc. */
#include "userprog/tss.h"      /* tss_update */
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"     /* FLAG_* constants */
#include "threads/thread.h"
#include "threads/vaddr.h"     /* PHYS_BASE */
#include "threads/interrupt.h" /* if_ */
#include "threads/init.h"      /* power_off() */

/* Headers not yet used that you may need for various reasons. */
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

#include "userprog/flist.h"
#include "userprog/plist.h"
#include "userprog/setup-argv.h"
/* HACK defines code you must remove and implement in a proper way */
#define HACK

struct map process_list; //plist
static struct lock cleanup_lock; /* Needs lock as map list is created by map.h (no special functionality for closing files)*/
/* This function is called at boot time (threads/init.c) to initialize
 * the process subsystem. */
void process_init(void)
{
	map_init(&process_list);
	lock_init(&cleanup_lock);
}

/* This function is currently never called. As thread_exit does not
 * have an exit status parameter, this could be used to handle that
 * instead. Note however that all cleanup after a process must be done
 * in process_cleanup, and that process_cleanup are already called
 * from thread_exit - do not call cleanup twice! */
void process_exit(int status)
{
	struct processInfo *p = map_find_associative(&process_list, thread_current()->tid)->value;
	p->exit_status = status;
	//debug("Exiting process: %d, status %d\n", p->pid, p->exit_status);
}

/* Print a list of all running processes. The list shall include all
 * relevant debug information in a clean, readable format. */
void process_print_list()
{
	plist_print(&process_list);
}

struct parameters_to_start_process
{
  char* command_line;
	int parent_pid;
	int ret_id;
	struct semaphore sema;
};

static void
start_process(struct parameters_to_start_process* parameters) NO_RETURN;

/* Starts a new proccess by creating a new thread to run it. The
   process is loaded from the file specified in the COMMAND_LINE and
   started with the arguments on the COMMAND_LINE. The new thread may
   be scheduled (and may even exit) before process_execute() returns.
   Returns the new process's thread id, or TID_ERROR if the thread
   cannot be created. */
int
process_execute (const char *command_line)
{
  char debug_name[64];
  int command_line_size = strlen(command_line) + 1;
  tid_t thread_id = -1;
  int  process_id = -1;

  /* LOCAL variable will cease existence when function return! */
  struct parameters_to_start_process arguments;

  debug("%s#%d: process_execute(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        command_line);

  /* COPY command line out of parent process memory */
  arguments.command_line = malloc(command_line_size);
  strlcpy(arguments.command_line, command_line, command_line_size);


  strlcpy_first_word (debug_name, command_line, 64);

	// Set parent
	arguments.parent_pid = thread_current()->tid;
	sema_init(&arguments.sema, 0);
	arguments.ret_id = -1;
	/* SCHEDULES function `start_process' to run (LATER) */
  thread_id = thread_create (debug_name, PRI_DEFAULT,
                             (thread_func*)start_process, &arguments);

  process_id = thread_id;

  /* AVOID bad stuff by turning off. YOU will fix this! */
  if(process_id != -1)
	{
		sema_down(&arguments.sema);
		process_id = arguments.ret_id; /* since we may create thread but fail to start process. */
	}

  /* WHICH thread may still be using this right now? */
  free(arguments.command_line);

  debug("%s#%d: process_execute(\"%s\") RETURNS %d\n",
        thread_current()->name,
        thread_current()->tid,
        command_line, process_id);

  /* MUST be -1 if `load' in `start_process' return false */
  return process_id;
}

/* A thread function that loads a user process and starts it
   running. */
static void
start_process (struct parameters_to_start_process* parameters)
{
  /* The last argument passed to thread_create is received here... */
  struct intr_frame if_;
  bool success;

  char file_name[64];
  strlcpy_first_word (file_name, parameters->command_line, 64);

  debug("%s#%d: start_process(\"%s\") ENTERED\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;

  success = load (file_name, &if_.eip, &if_.esp);

  debug("%s#%d: start_process(...): load returned %d\n",
        thread_current()->name,
        thread_current()->tid,
        success);

  if (success)
  {
    /* We managed to load the new program to a process, and have
       allocated memory for a process stack. The stack top is in
       if_.esp, now we must prepare and place the arguments to main on
       the stack. */

    /* A temporary solution is to modify the stack pointer to
       "pretend" the arguments are present on the stack. A normal
       C-function expects the stack to contain, in order, the return
       address, the first argument, the second argument etc. */

    //HACK if_.esp -= 12; /* Unacceptable solution. */
		if_.esp = setup_main_stack(parameters->command_line, if_.esp);
		plist_insert(&process_list, plist_create_process(thread_current()->tid, parameters->parent_pid), thread_current()->tid);
		//debug("Added process: %d, Child to: %d\n", thread_current()->tid, parameters->parent_pid);
		parameters->ret_id = thread_current()->tid;

		    /* The stack and stack pointer should be setup correct just before
       the process start, so this is the place to dump stack content
       for debug purposes. Disable the dump when it works. */

    //dump_stack ( PHYS_BASE + 15, PHYS_BASE - if_.esp + 16 );
  }

  debug("%s#%d: start_process(\"%s\") DONE\n",
        thread_current()->name,
        thread_current()->tid,
        parameters->command_line);

	sema_up(&parameters->sema);
  /* If load fail, quit. Load may fail for several reasons.
     Some simple examples:
     - File doeas not exist
     - File do not contain a valid program
     - Not enough memory
  */
  if ( ! success )
  {
    thread_exit ();
  }

  /* Start the user process by simulating a return from an interrupt,
     implemented by intr_exit (in threads/intr-stubs.S). Because
     intr_exit takes all of its arguments on the stack in the form of
     a `struct intr_frame', we just point the stack pointer (%esp) to
     our stack frame and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}

/* Wait for process `child_id' to die and then return its exit
   status. If it was terminated by the kernel (i.e. killed due to an
   exception), return -1. If `child_id' is invalid or if it was not a
   child of the calling process, or if process_wait() has already been
   successfully called for the given `child_id', return -1
   immediately, without waiting.

   This function will be implemented last, after a communication
   mechanism between parent and child is established. */
int
process_wait (int child_id)
{
  int status = -1;
  struct thread *cur = thread_current ();

  debug("%s#%d: process_wait(%d) ENTERED\n",
        cur->name, cur->tid, child_id);

	struct processInfo *p = map_find(&process_list, child_id);

	if(p != NULL && cur->tid == p->parent_pid && !p->waiting)
	{
		if(p->alive)
		{
			p->waiting = true;
			sema_down(&p->sema);
		}
		status = p->exit_status;
	}
  debug("%s#%d: process_wait(%d) RETURNS %d\n",
        cur->name, cur->tid, child_id, status);

  return status;
}

/* Free the current process's resources. This function is called
   automatically from thread_exit() to make sure cleanup of any
   process resources is always done. That is correct behaviour. But
   know that thread_exit() is called at many places inside the kernel,
   mostly in case of some unrecoverable error in a thread.

   In such case it may happen that some data is not yet available, or
   initialized. You must make sure that nay data needed IS available
   or initialized to something sane, or else that any such situation
   is detected.
*/

void
process_cleanup (void)
{
  struct thread  *cur = thread_current ();
  uint32_t       *pd  = cur->pagedir;
  int status = -1;

	debug("%s#%d: process_cleanup() ENTERED\n", cur->name, cur->tid);

  /* Later tests DEPEND on this output to work correct. You will have
   * to find the actual exit status in your process list. It is
   * important to do this printf BEFORE you tell the parent process
   * that you exit.  (Since the parent may be the main() function,
   * that may sometimes poweroff as soon as process_wait() returns,
   * possibly before the printf is completed.)
   */

	 /* Set status to exit_status. */
	 struct processInfo *p = map_find(&process_list, cur->tid);
	 if(p != NULL)
		 status = p->exit_status;
  printf("%s: exit(%d)\n", thread_name(), status);

	lock_acquire(&cleanup_lock);																			/* Synch lock */
	for(size_t fd = 2; fd < list_size(&cur->f_map.content) + 2; ++fd)	/* Close all open files in file-map, starts at fd = 2 since it's the first key(fd) */
		filesys_close((struct file*)map_find(&cur->f_map, fd));					/* Type cast might be redundant. */
	lock_release(&cleanup_lock);																			/* Free lock */
	free_all_mem(&cur->f_map); 																				/* Free all pointers in f_map */

	if(p != NULL)
	{
		//p->waiting = false;
		plist_remove(&process_list, cur->tid);													/* Remove process if parent from active list (if parent is dead), fix new search problem. */
		sema_up(&p->sema);																							/* Release sema of process. If wait(p) we sema down for waiting and sema up in cleanup */
	}

	/* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  if (pd != NULL)
  {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
  }


  debug("%s#%d: process_cleanup() DONE with status %d\n",
        cur->name, cur->tid, status);
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}
