/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

void itoa(int a, char *b);

int strlen(char *a);

int write(int fd, char *buffer, int size);

void perror(void);

int gettime(void);

int getpid(void);

int fork(void);

void exit(void);

void block(void);

int unblock(int pid);

int read(char* b, int maxchars);

int getchar(void);

#endif  /* __LIBC_H__ */
