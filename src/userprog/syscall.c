#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:

   int sys_read_arg_count = argc[ SYS_READ ];

   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls, in order as defined in syscall-nr.h */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1,
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static void
syscall_handler (struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
	int sys_read_arg_count = argc[ esp[0] ];

  switch ( esp[0] /* retrive syscall number */ ) // syscall number top of stack
  {
		case SYS_HALT:
		{
			debug("CALLED: SYS_HALT\n");
			power_off();
			break;
		}
		case SYS_EXIT:
		{
			debug("CALLED: SYS_EXIT, CODE (%d)\n", (int)esp[1]);
			process_exit((int)esp[1]);  // Set exit code for process.
			thread_exit(); 							// Close current thread.
			break;
		}
		case SYS_EXEC:
			break;
		case SYS_WAIT:
			break;
		case SYS_CREATE:
			break;
		case SYS_REMOVE:
			break;
		case SYS_OPEN:
			break;
		case SYS_FILESIZE:
			break;
		case SYS_READ:
		{
			int fd = esp[1];
			char *buf = (char*)esp[2];
			int buf_s = esp[3];
			if(fd == STDIN_FILENO)
			{
				for(int i = 0; i < buf_s; ++i)
				{
					uint8_t tmp = input_getc();

					if(tmp == '\r')
						tmp = '\n';
					buf[i] = tmp;
					putbuf(&buf[i], 1);
				}
				f->eax = buf_s;
			}
			else
				f->eax = -1;
			break;
		}
		case SYS_WRITE:
		{
			int fd = esp[1];
			char *buf = (char*)esp[2];
			int buf_s = esp[3];

			if(fd == STDOUT_FILENO)
			{
				putbuf(buf, buf_s);
				f->eax = buf_s;
			}
			else
				f->eax = -1;
			break;
		}
		case SYS_SEEK:
			break;
		case SYS_TELL:
			break;
		case SYS_CLOSE:
			break;
		case SYS_MMAP:
			break;
		case SYS_MUNMAP:
			break;
		case SYS_CHDIR:
			break;
		case SYS_MKDIR:
			break;
		case SYS_READDIR:
			break;
		case SYS_ISDIR:
			break;
		case SYS_INUMBER:
			break;
    default:
    {
      printf ("Executed an unknown system call!\n");

      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
    }
  }
}
