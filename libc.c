/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno = 0;

static char *error_messages[] = {
    "Unknown error",                    // 0
    "Operation not permitted",          // 1  EPERM
    "No such file or directory",        // 2  ENOENT
    "No such process",                  // 3  ESRCH
    "Interrupted system call",          // 4  EINTR
    "I/O error",                        // 5  EIO
    "No such device or address",        // 6  ENXIO
    "Argument list too long",           // 7  E2BIG
    "Exec format error",                // 8  ENOEXEC
    "Bad file descriptor",              // 9  EBADF
    "No child processes",               // 10 ECHILD
    "Try again",                        // 11 EAGAIN
    "Out of memory",                    // 12 ENOMEM
    "Permission denied",                // 13 EACCES
    "Bad address",                      // 14 EFAULT
    "Block device required",            // 15 ENOTBLK
    "Device or resource busy",          // 16 EBUSY
    "File exists",                      // 17 EEXIST
    "Cross-device link",                // 18 EXDEV
    "No such device",                   // 19 ENODEV
    "Not a directory",                  // 20 ENOTDIR
    "Is a directory",                   // 21 EISDIR
    "Invalid argument",                 // 22 EINVAL
    "File table overflow",              // 23 ENFILE
    "Too many open files",              // 24 EMFILE
    "Not a typewriter",                 // 25 ENOTTY
    "Text file busy",                   // 26 ETXTBSY
    "File too large",                   // 27 EFBIG
    "No space left on device",          // 28 ENOSPC
    "Illegal seek",                     // 29 ESPIPE
    "Read-only file system",            // 30 EROFS
    "Too many links",                   // 31 EMLINK
    "Broken pipe",                      // 32 EPIPE
    "Math argument out of domain",      // 33 EDOM
    "Math result not representable",    // 34 ERANGE
    "Resource deadlock would occur",    // 35 EDEADLK
    "File name too long",               // 36 ENAMETOOLONG
    "No record locks available",        // 37 ENOLCK
    "Function not implemented",         // 38 ENOSYS
};

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
 
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror(void)
{
	char* msg = error_messages[errno];
	write(1, msg, strlen(msg));
}







