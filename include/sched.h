/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;
  DWord kernel_esp;		/* Position of the stack pointer in kernel stack. */
  struct list_head list;	/* List of tasks. */
  enum state_t state;		/* State of the process (ST_RUN, ST_READY, ST_BLOCKED) */
  int quantum;			/* Quantum of the process */
  int pending_unblocks;  /* Account for any pending unblock operation received. */
  struct list_head children;		/* List of child processes. */
  struct list_head sibling;		/* List of sibling processes. */
  struct task_struct * parent;	/* Pointer to the parent process. */
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

extern char initial_stack[KERNEL_STACK_SIZE];
#define INITIAL_ESP             (DWord) &initial_stack[KERNEL_STACK_SIZE]

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

void task_switch(union task_union *new); /* Canvia l'execució al proces new */

void inner_task_switch(union task_union *new); /* Canvia l'execució al proces new sense tocar el stack de sistema */

void update_sched_data_rr (void);

int needs_sched_rr (void);

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue); 

void sched_next_rr(void);

void schedule(void);

int get_quantum(struct task_struct *t);

void set_quantum(struct task_struct *t, int new_quantum);

struct task_struct * current();

struct task_struct *list_head_to_task_struct(struct list_head *l);

extern struct task_struct * init_task; /* Pointer to the initial task (task 1) */

extern struct task_struct * idle_task; /* Pointer to the idle task */

extern struct list_head readyqueue; /* Queue of ready processes */

extern int next_pid;

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

#endif  /* __SCHED_H__ */
