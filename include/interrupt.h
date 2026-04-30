/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
#define CIRCULAR_BUFFER_CHARACTERS 128
#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

struct circular_buffer {
	char data[CIRCULAR_BUFFER_CHARACTERS];	// char que contiene los datos del circular buffer.
	int head;	// pos de la entrada de caracteres.
	int tail;	// pos de caracteres pendientes. 
	int count;	// caracteres leidos.

};

extern struct circular_buffer keyboard_buffer; 

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

int add_circular_buffer(char c, struct circular_buffer *b);
int del_circular_buffer(char *c, struct circular_buffer *b);

void setIdt();

#endif  /* __INTERRUPT_H__ */
