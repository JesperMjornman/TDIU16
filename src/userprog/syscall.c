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
#include "userprog/plist.h"
#include "devices/input.h"
#include "devices/timer.h"

static void syscall_handler (struct intr_frame *);
static void sys_seek (int fd, unsigned position);
static void sys_close(int fd);
static void sys_exit (int status);
static void sys_halt (void);
static void sys_plist(void);

static int sys_read (int fd, char *buf, int len);
static int sys_write(int fd, const char *buf, int len);
static int sys_open  (const char *fname);
static int sys_create(const char *fname, unsigned init_size);
static int sys_remove(const char *fname);
static int sys_filesize(int fd);

static unsigned sys_tell(int fd);

static bool valid_ptr(void *ptr);
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
  1, 0
};

static void
syscall_handler (struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
	//int sys_read_arg_count = argc[ esp[0] ];
  switch ( esp[0] /* retrive syscall number */ )
  {
		case SYS_HALT:
			sys_halt();
			break;
		case SYS_EXIT:
			sys_exit((int)esp[1]);
			break;
		case SYS_EXEC:
			f->eax = process_execute((const char*)esp[1]);
			break;
		case SYS_WAIT:
			f->eax = process_wait((int)esp[1]);
			break;
		case SYS_CREATE:
			f->eax = sys_create((const char*)esp[1], (unsigned)esp[2]);
			break;
		case SYS_REMOVE:
			f->eax = sys_remove((const char*)esp[1]);
			break;
		case SYS_OPEN:
			f->eax = sys_open((const char*)esp[1]);
			break;
		case SYS_FILESIZE:
			f->eax = sys_filesize((int)esp[1]);
			break;
		case SYS_READ:
			f->eax = sys_read((int)esp[1], (char*)esp[2], (int)esp[3]);
			break;
		case SYS_WRITE:
			f->eax = sys_write((int)esp[1], (const char*)esp[2], (int)esp[3]);
			break;
		case SYS_SEEK:
			sys_seek((int)esp[1], (unsigned)esp[2]);
			break;
		case SYS_TELL:
			f->eax = sys_tell((int)esp[1]);
			break;
		case SYS_CLOSE:
			sys_close((int)esp[1]);
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
		case SYS_SLEEP:
			timer_msleep((int)esp[1]);
			break;
		case SYS_PLIST:
			sys_plist();
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

static int sys_read(int fd, char *buf, int len)
{
	int n_char = 0;

	if(fd == STDIN_FILENO)
	{
		while(n_char != len)
		{
			uint8_t tmp = input_getc();

			if(tmp == '\r')
				tmp = '\n';
			else if(tmp < 32) 	/* To handle special buttons when asking for input. */
				continue;
			else if(tmp == 127) /* Handle backspace for deletion of characters in buffer (Note: only in terminal!) */
			{
				if(n_char == 0)
					continue;

				buf[--n_char] = 0;
				putbuf("\b \b", 3);
				continue;
			}

			buf[n_char] = tmp;
			putbuf(&buf[n_char++], 1);
		}
		return len;
	}
	else if(fd > 1)
	{
		struct file *fp = map_find(&thread_current()->f_map, fd);
		if(fp == NULL) /* Is the file open? */
			return -1;

		return file_read(fp, buf, len);
	}
	else
		return -1;
}

static int sys_write(int fd, const char *buf, int len)
{
	if(fd == STDOUT_FILENO)
	{
		putbuf(buf, len);
		return len;
	}
	else if(fd > 1)
	{
		/*
		   Find file in current thread's file map
			 Not found -> return -1
		 */
		struct file *fp = map_find(&thread_current()->f_map, fd);
		if(fp == NULL)
			return -1;

		return file_write(fp, buf, len);
	}
	else
		return -1;
}

static int sys_open(const char *fname)
{
	if(!valid_ptr((void*)fname))
		return -1;
	struct file *fp = filesys_open(fname);
	if(fp == NULL)
		return -1;
	/* Save fp to current thread's open file map */
	int fd = map_insert(&thread_current()->f_map, fp);
	if(fd == -1) /* On failure of insertion */
		filesys_close(fp);

	return fd;
}

static int sys_create(const char *fname, unsigned init_size)
{
	if(!valid_ptr((void*)fname))
		return -1;
	return filesys_create(fname, init_size);
}

static void sys_close(int fd)
{
	if(fd > 1) /* fd < 2 is STDIN/STDOUT, use to avoid errors. */
	{
		filesys_close(map_find(&thread_current()->f_map, fd));
		/* Remove from f_map as file is closed */
		map_remove(&thread_current()->f_map, fd);
	}
}

static int sys_remove(const char *fname)
{
	if(!valid_ptr((void*)fname))
		return -1;
	return filesys_remove(fname);
}

static int sys_filesize(int fd)
{
	if(fd < 2)
		return -1;

	struct file *fp = map_find(&thread_current()->f_map, fd);
	if(fp == NULL)
		return -1;

	return file_length(fp);
}

static unsigned sys_tell(int fd)
{
	if(fd < 2)
		return -1;

	struct file *fp = map_find(&thread_current()->f_map, fd);
	if(fp == NULL)
		return -1;

	return file_tell(fp);
}

static void sys_seek(int fd, unsigned position)
{
	if(fd > 1) /* Has to be a file */
	{
		struct file *fp = map_find(&thread_current()->f_map, fd);
		if(fp != NULL)
			file_seek(fp, position);
	}
}

static void sys_halt(void)
{
	free_all_mem(&process_list);
	power_off();
}

static void sys_exit(int status)
{
	process_exit(status);  				// Set exit code for process.
	thread_exit(); 							// Close current thread.
}

static void sys_plist(void)
{
	plist_print(&process_list);
}

static bool valid_ptr(void *ptr)
{
	return(ptr != NULL && ptr < PHYS_BASE);
}
