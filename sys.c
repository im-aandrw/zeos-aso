/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>
#include <errno.h>
#define LECTURA 0
#define ESCRIPTURA 1
extern unsigned int zeos_ticks;
char nbuffer[256];          

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_write(int fd, char *buffer, int size)
{
  int error = check_fd(fd, ESCRIPTURA);
  if (error < 0) return error;
  if (buffer == NULL) return -EFAULT;
  if (size < 0) return -EINVAL;
  int bytes = 0;

  while (size > 0) {
    int nsize = size;
    if (nsize > (int)sizeof(nbuffer)) nsize = (int)sizeof(nbuffer);
    if (!access_ok(VERIFY_READ, buffer, nsize)) return -EFAULT;
    int r = copy_from_user(buffer, nbuffer, nsize);
    if (r < 0) return -EFAULT;   

    sys_write_console(nbuffer, nsize);

    buffer += nsize;
    size   -= nsize;
    bytes  += nsize;
  }
  return bytes;
}

int sys_gettime()
{
	if (zeos_ticks < 0) return -EINTR;
	return zeos_ticks;
}

