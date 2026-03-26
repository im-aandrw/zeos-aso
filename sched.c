/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <hardware.h>

char initial_stack[KERNEL_STACK_SIZE]; // Space for the initial system stack
struct task_struct * init_task;
struct task_struct * idle_task;

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
	// Page Directory:
	// función alloc_frame(): da el número de un frame libre y lo marca como ocupado.
	int Dir = alloc_frame ();
	// 17 << 12  =  17 * 4096  =  dirección física 0x11000.
	page_table_entry *DirAddress = (page_table_entry *) (Dir << 12);
	// No crea System Page Table, reutiliza la de init().
	page_table_entry *SPTAddress = get_PT(init_task);
	int SPT = (int)SPTAddress >> 12;
	
	set_ss_pag(SPTAddress, Dir, Dir, 0);  // 1. mapear Dir
	clear_page_table(DirAddress);          // 2. limpiar Dir
	set_ss_pag(DirAddress, 0, SPT, 0);    // 3. conectar Dir con SPT
	// No necesita parte de usuario.
	
	// Reserva 1 frame → lo usa como task_struct (PCB del proceso)
	int frame = alloc_frame();
	union task_union *new_task = (union task_union*) (frame << 12);
	set_ss_pag(SPTAddress, frame, frame, 0);
	// Preparar el proceso para task_switch.
	// Task switch tendrá esto como direccion de ret
	new_task->stack[KERNEL_STACK_SIZE-1] = (unsigned long)cpu_idle;
	// Task switch hará un pop de este contenido.
	new_task->stack[KERNEL_STACK_SIZE-2] = 0;
	// actualiza el esp para que apunte al 0.
	new_task->task.kernel_esp = (unsigned long)&(new_task->stack[KERNEL_STACK_SIZE-2]);
	
	new_task->task.PID = 0;
	new_task->task.dir_pages_baseAddr = DirAddress;
	idle_task = &(new_task->task);
}

// init process.
void init_task1(void)
{
	// Page Directory:
	// función alloc_frame(): da el número de un frame libre y lo marca como ocupado.
	int Dir = alloc_frame ();
	// 17 << 12  =  17 * 4096  =  dirección física 0x11000.
	page_table_entry *DirAddress = (page_table_entry *) (Dir << 12);
	clear_page_table(DirAddress);
	// System Page Table.
	int SPT = alloc_frame ();
	page_table_entry *SPTAddress = (page_table_entry *) (SPT << 12);
	clear_page_table(SPTAddress);
	set_kernel_pages(SPTAddress);
	// User Page Table.
	int UPT = alloc_frame ();
	page_table_entry *UPTAddress = (page_table_entry *) (UPT << 12);
	clear_page_table(UPTAddress);
	set_user_pages(UPTAddress);
	// El frame número SPT existe en memoria física, y quiero poder acceder a él desde el kernel usando la misma dirección lógica.
	set_ss_pag(SPTAddress, Dir, Dir, 0);
	set_ss_pag(SPTAddress, SPT, SPT, 0);
	set_ss_pag(SPTAddress, UPT, UPT, 0);
	
	// Conecta todo: Directory apunta a las dos Page Tables.
	set_ss_pag(DirAddress, 0, SPT, 0);
	set_ss_pag(DirAddress, 1, UPT, 1);
	
	// Reserva 1 frame → lo usa como task_struct (PCB del proceso)
	int frame = alloc_frame();
	union task_union *new_task = (union task_union*) (frame << 12);
	set_ss_pag(SPTAddress, frame, frame, 0);
	
	// 6. Configura el proceso (PID, stack, TSS...)
	new_task->task.PID = 1;
	tss.esp0 = (unsigned long)&(new_task->stack[KERNEL_STACK_SIZE]);
	new_task->task.dir_pages_baseAddr = DirAddress;
	
	// Activa este directorio en el hardware (registro CR3).
	set_cr3(DirAddress);
	
	init_task = &(new_task->task);
}


void init_sched()
{
	struct list_head readyqueue;
}

void inner_task_switch(union task_union *new)
{
	// tss.esp0 = apunta al stack del nuevo proceso.
	tss.esp0 = (unsigned long)&(new->stack[KERNEL_STACK_SIZE]);
	// avisamos al hardware de que ahora tiene que usar el Dir del nuevo proceso.
	set_cr3(new->task.dir_pages_baseAddr);
	
	// En assembly inline de GCC, para escribir en una variable C desde assembly se usa %0 como placeholder.
	__asm__ __volatile__(
	"movl %%ebp, %0\n\t"
	"movl %1, %%esp\n\t"
	"pop %%ebp\n\t"
	"ret\n\t"
	: "=m" (current()->kernel_esp) /* variables C donde escribimos */
	: "m" (new->task.kernel_esp) /* variables C de donde leemos */
	);
}

void task_switch(union task_union *new)
{
	__asm__ __volatile__(
	"pushl %esi\n\t"
	"pushl %edi\n\t"
	"pushl %ebx\n\t"
	);
	inner_task_switch(new);
	__asm__ __volatile__(
	"popl %ebx\n\t"
	"popl %edi\n\t"
	"popl %esi\n\t"
	);
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

