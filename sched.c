/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <hardware.h>
#include <list.h>
#include <devices.h>

char initial_stack[KERNEL_STACK_SIZE]; // Space for the initial system stack
struct task_struct * init_task;
struct task_struct * idle_task;
struct list_head readyqueue; /* Queue of ready processes */

int next_pid = 2;

int remaining_quantum; // Variable to store the remaining quantum of the current process
#define DEFAULT_QUANTUM 10

void cpu_idle(void)
{
	__sti();
	while(1)
	{
	;
	}
}

void init_idle (void)
{	
	// Create a new address space for the idle task and set its fields
	int Dir = alloc_frame ();
	page_table_entry *DirAddress = (page_table_entry *) (Dir << 12);
	clear_page_table(DirAddress);

	// Map the system page table of the idle task in its page directory
	page_table_entry *SPTAdress = get_PT(init_task);
	// Convert the address of the system page table to a page frame number and set the corresponding entry in the page directory of the idle task
	DWord addr = (DWord) SPTAdress;
	int SPT_frame = addr >> 12;
	set_ss_pag(SPTAdress, Dir, Dir, 0);
	set_ss_pag(DirAddress, 0, SPT_frame, 0);

	// Store the address of the code that will execute (address of the function cpu_idle)
	int TU = alloc_frame();
	union task_union *TUAddress = (union task_union *) (TU << 12);
	set_ss_pag(SPTAdress, TU, TU, 0);
	TUAddress -> stack[KERNEL_STACK_SIZE-1] = (unsigned long) cpu_idle;
	TUAddress -> stack[KERNEL_STACK_SIZE-2] = 0; // ebp = 0
	TUAddress -> task.kernel_esp = (DWord) &(TUAddress->stack[KERNEL_STACK_SIZE-2]);

	// Allocate a new task_struct for the idle task and set its fields
	struct task_struct *TS = &(TUAddress->task);
	TS->PID = 0;
	TS->dir_pages_baseAddr = DirAddress;
	TS->state = ST_READY;
	INIT_LIST_HEAD(&(TS->list));
	TS->quantum = DEFAULT_QUANTUM;
	TS->pending_unblocks = 0;
	INIT_LIST_HEAD(&(TS->children));
	INIT_LIST_HEAD(&(TS->sibling));
	TS->parent = NULL;

	// Update the pointer to the idle task (idle_task) to point to the idle task
	idle_task = &(TUAddress->task);
}	

void init_task1(void)
{
	// Allocate a page for the Page Directory of the initial task and set its fields
	int Dir = alloc_frame ();
	page_table_entry *DirAddress = (page_table_entry *) (Dir << 12);
	clear_page_table(DirAddress);

	// Allocate a page to store system mapping of the initial task and set its fields
	int SPT = alloc_frame ();
	page_table_entry *SPTAdress = (page_table_entry *) (SPT << 12);
	clear_page_table(SPTAdress);
	set_kernel_pages(SPTAdress);

	// Allocate a page to store user mapping of the initial task and set its fields
	int UPT = alloc_frame ();
	page_table_entry *UPTAdress = (page_table_entry *) (UPT << 12);
	clear_page_table(UPTAdress);
	set_user_pages(UPTAdress);

	// Map Direcctoy, and System and User Page Tables in the system page table of the initial task
	set_ss_pag(SPTAdress, Dir, Dir, 0);
	set_ss_pag(SPTAdress, SPT, SPT, 0);
	set_ss_pag(SPTAdress, UPT, UPT, 0);

	// Assign System and User Page Tables to the corresponding fields of the Page Directory of the initial task
	set_ss_pag(DirAddress, 0, SPT, 0);
	set_ss_pag(DirAddress, 1, UPT, 1);

	// Allocate a frame for the task union of the initial task and set its fields
	int TU = alloc_frame();
	union task_union *TUAddress = (union task_union *) (TU << 12);
	set_ss_pag(SPTAdress, TU, TU, 0);
	TUAddress->task.PID = 1;
	TUAddress->task.state = ST_RUN;
	INIT_LIST_HEAD(&(TUAddress->task.list));
	TUAddress->task.quantum = DEFAULT_QUANTUM;
	TUAddress->task.pending_unblocks = 0;
	INIT_LIST_HEAD(&(TUAddress->task.children));
	INIT_LIST_HEAD(&(TUAddress->task.sibling));
	TUAddress->task.parent = NULL;

	// Update TSS for the initial task
	tss.esp0 = KERNEL_ESP(TUAddress);
	writeMSR(0x175, KERNEL_ESP(TUAddress));

	// Initialize field dir_pages_baseAddr of the initial task with the address of its Page Directory
	TUAddress -> task.dir_pages_baseAddr = DirAddress;
	
	// Set its page directory as the current page directory
	set_cr3(DirAddress);

	// Update the pointer to the initial task (init_task) to point to the initial task
	init_task = &(TUAddress->task);
	
	remaining_quantum = get_quantum(init_task);
}


void init_sched()
{
	INIT_LIST_HEAD(&readyqueue);
	INIT_LIST_HEAD(&blocked);
}


void task_switch(union task_union *new)
{
	// Save registers ESI EDI EBX
	__asm__ __volatile__("pushl %esi");
	__asm__ __volatile__("pushl %edi");
	__asm__ __volatile__("pushl %ebx");
	
	inner_task_switch(new);

	// Restore registers ESI EDI EBX
	__asm__ __volatile__("popl %ebx");
	__asm__ __volatile__("popl %edi");
	__asm__ __volatile__("popl %esi");

}


void inner_task_switch(union task_union *new)
{
	// Update TSS for the new task
	tss.esp0 = KERNEL_ESP(new);
	writeMSR(0x175, KERNEL_ESP(new));

	// Switch page directory (CR3) to the page directory of the new task
	set_cr3(get_DIR(&(new->task)));

	// Store the current value of EBP register in the PCB
	__asm__ __volatile__("movl %%ebp, %0" : "=r" (current()->kernel_esp));

	// Set ESP register to point to the stored value in the new PCB
	__asm__ __volatile__("movl %0, %%esp":: "r" (new->task.kernel_esp));

	// Restore the EBP register from the stack
	__asm__ __volatile__("popl %ebp");
	__asm__ __volatile__("ret");
}



/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t)
{
       return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t)
{
       return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
	return list_entry(l, struct task_struct, list);
}


void update_sched_data_rr (void)
{
	remaining_quantum--;
}

int needs_sched_rr (void)
{
	if (current() == idle_task && !list_empty(&readyqueue)) return 1;
	if (remaining_quantum <= 0 && !list_empty(&readyqueue)) return 1;
	return 0;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue)
{
	// If the process is changing from ready to running, we need to delete it from the ready queue. 
	// If the process is changing to ready, we need to add it to the ready queue.
	// Finally, we need to update the state of the process.
	if (t->state != ST_RUN) list_del(&(t->list));
	if (dst_queue != NULL) list_add_tail(&(t->list), dst_queue);
	if (dst_queue == NULL) t->state = ST_RUN;
	else if (dst_queue == &readyqueue) t->state = ST_READY;
	else t->state = ST_BLOCKED;
}

void sched_next_rr(void)
{
	// If the ready queue is empty, we will execute the idle task. Otherwise, we will execute the first process in the ready queue.
	struct task_struct *next;
	if (list_empty(&readyqueue)) {
		next = idle_task;
		next->state = ST_RUN; // Update the state of the idle task to running. We can't use update_process_state_rr() because the idle task is not in the ready queue.
	}
		else {
			// Get the first process in the ready queue and update its state to running
			struct list_head *firste = list_first(&readyqueue); // Get the first element of the ready queue
			next = list_head_to_task_struct(firste); // Get the first process in the ready queue
			update_process_state_rr(next, NULL);
		}
	remaining_quantum = get_quantum(next);
	task_switch((union task_union *) next);
}

void schedule(void)
{
	// Update the remaining quantum of the current process and check if we need to reschedule. 
	// If so, call sched_next_rr() to choose the next process to execute.
	update_sched_data_rr();
	if (needs_sched_rr()) {
		if (current() != idle_task) update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}

}

int get_quantum(struct task_struct *t)
{
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum)
{
	t->quantum = new_quantum;
}
