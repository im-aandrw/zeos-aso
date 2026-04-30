#ifndef DEVICES_H__
#define  DEVICES_H__

#include <list.h>

extern struct list_head blocked;


int sys_write_console(char *buffer,int size);
#endif /* DEVICES_H__*/
