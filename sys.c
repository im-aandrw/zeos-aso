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
extern void ret_from_fork(void);
extern struct circular_buffer keyboard_buffer;
extern int del_circular_buffer(char *c, struct circular_buffer *b);

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

int sys_getpid()
{
  return current()->PID;
}

int sys_fork()
{
  // Get a free task struck for the process and if there is no free space, return an error
  int frame = alloc_frame();
  if (frame < 0) return -EAGAIN;
  union task_union *child = (union task_union *)(frame << 12);
  set_ss_pag(get_PT(current()), frame, frame, 0);

  // Inherit the parent's task union
  union task_union *parent = (union task_union *)current();
  copy_data(parent, child, sizeof(union task_union));

  // Initialize child address space
  // Get structure to store the Page Directory of the child and if there is no free space,
  //return an error and free the page allocated for the task union of the child process
  int ChildsDir = alloc_frame();
  if (ChildsDir < 0) {
    free_frame(frame);
    del_ss_pag(get_PT(current()), frame);
    return -EAGAIN;
  }
  page_table_entry *ChildsDirAddress = (page_table_entry *)(ChildsDir << 12);
  set_ss_pag(get_PT(current()), ChildsDir, ChildsDir, 0);
  clear_page_table(ChildsDirAddress);
  // Initialize field dir_pages_baseAddr of the child process with the address of its Page Directory
  child->task.dir_pages_baseAddr = ChildsDirAddress;
  // System pages are shared with the parent process
  ChildsDirAddress[0].entry = parent->task.dir_pages_baseAddr[0].entry;
  // Get structure to store the User Page Table of the child and if there is no free space, 
  //return an error and free the page allocated for the Page Directory of the child process
  int CUPT = alloc_frame();
  if (CUPT < 0) {
    free_frame(ChildsDir);
    free_frame(frame);
    del_ss_pag(get_PT(current()), ChildsDir);
    del_ss_pag(get_PT(current()), frame);
    return -EAGAIN;
  }
  page_table_entry *CUPTAddress = (page_table_entry *)(CUPT << 12);
  set_ss_pag(get_PT(current()), CUPT, CUPT, 0);
  clear_page_table(CUPTAddress);
  // Map the User Page Table of the child process in the second entry of its Page Directory and set its fields
  set_ss_pag(ChildsDirAddress, 1, CUPT, 1);
  // Copy the content of the User Page Table of the parent process to the User Page Table of the child process
  // Get the address of the User Page Table of the parent process
  page_table_entry *PUPTAddress = (page_table_entry *)(parent->task.dir_pages_baseAddr[1].bits.pbase_addr << 12);
  // Copy the content of the User Page Table of the parent process to the User Page Table of the child process
  for (int i = NUM_PAG_DATA; i < NUM_PAG_DATA+NUM_PAG_CODE; ++i) {
    CUPTAddress[i] = PUPTAddress[i];
  }
  // Search frames in which to map logical pages for data+stack of the child process and if there is no free space, return an error
  for (int i = 0; i < NUM_PAG_DATA; ++i) {
    int frameDS = alloc_frame();
    if (frameDS < 0) {
      for (int j = 0; j < i; ++j) {
        free_frame(CUPTAddress[j].bits.pbase_addr);
      }
      free_frame(CUPT);
      free_frame(ChildsDir);
      free_frame(frame);
      del_ss_pag(get_PT(current()), CUPT);
      del_ss_pag(get_PT(current()), ChildsDir);
      del_ss_pag(get_PT(current()), frame);
      return -EAGAIN;
    }
    // Map the frame found in the previous step in the corresponding entry of the User Page Table of the child process and set its fields
    set_ss_pag(CUPTAddress, i, frameDS, 1);
  }

  // Inherit user data. User data+stack pages from the parent process are copied to the child process.
  // For each page, copy the content of the page in the parent process to the corresponding page in the child process 
  // and if there is an error, return an error and free the pages allocated for the child process
  int TMP_BASE = NUM_PAG_DATA+NUM_PAG_CODE;
  for (int i = 0; i < NUM_PAG_DATA; ++i) {
    // Map a temporary page in the user page table of the parent process to copy the content of the page in the parent process
    set_ss_pag(PUPTAddress, TMP_BASE+i, CUPTAddress[i].bits.pbase_addr, 1);
    copy_data((void *)(L_USER_START+(i*PAGE_SIZE)), (void *)(L_USER_START + (TMP_BASE+i)*PAGE_SIZE), PAGE_SIZE);
    // Unmap the temporary page in the system page table of the parent process
    del_ss_pag(PUPTAddress, TMP_BASE+i);
  }
  // Flush the TLB
  set_cr3(current()->dir_pages_baseAddr);

  // Initialize the fields of the PCB of the child process that are different from the corresponding fields of the PCB of the parent process
  // Assign a new PID to the child process
  child->task.PID = next_pid++;
  child->task.parent = &parent->task;
  child->task.pending_unblocks = 0;
  INIT_LIST_HEAD(&(child->task.children));
  INIT_LIST_HEAD(&(child->task.sibling));
  list_add_tail(&(child->task.sibling), &(parent->task.children));
  // Prepare the child stack so that a task_switch call on this PCB restores the process execution.
  // Get the base address of the parent and child task unions to calculate the value to initialize the EBP of the child process
  DWord parent_ebp;
  __asm__ __volatile__("movl %%ebp, %0" : "=r"(parent_ebp));

  DWord parent_base = (DWord) parent;
  DWord child_base  = (DWord) child;
  DWord child_ebp   = child_base + (parent_ebp - parent_base);
  // Forge the stack so inner_task_switch restores:
  //   pop %ebp -> 0
  //   ret      -> ret_from_fork
  // and then ret_from_fork returns to the copied return address of sys_fork,
  // which already remains at child_ebp + 4 in the copied stack frame.
  DWord *ksp = (DWord *)(child_ebp - 4);
  ksp[0] = 0;
  ksp[1] = (DWord) ret_from_fork;
  child->task.kernel_esp = (DWord) ksp;


  // Insert the child process in the ready queue and set its state to ST_READY
  child->task.state = ST_READY;
  list_add_tail(&(child->task.list), &readyqueue);

  // Return the PID of the child process
  return child->task.PID;
}

void sys_exit()
{
  // Free the data structures and resources of this process (physical memory, task_struct,
  // and so). It uses the free_frame function to free physical pages.
  struct task_struct *current_task = current();
  page_table_entry *dir = current_task->dir_pages_baseAddr;
  page_table_entry *user_PT = (page_table_entry *)(dir[1].bits.pbase_addr << 12);

  int dir_frame = ((DWord)dir) >> 12;
  int user_PT_frame = dir[1].bits.pbase_addr;
  int task_union_frame = ((DWord)current_task) >> 12;

  // Delete actual process form the children list of its parent process
  if (current_task->parent != NULL) list_del(&(current_task->sibling));

  // Move alive children of the actual process to be children of the idle process and actualize their parent field
  while(!list_empty(&(current_task->children))) {
    struct task_struct *child = list_entry(current_task->children.next, struct task_struct, sibling);
    list_del(&(child->sibling));
    list_add_tail(&(child->sibling), &(idle_task->children));
    child->parent = idle_task;
  }


  free_user_pages(user_PT);
  free_frame(user_PT_frame);
  free_frame(dir_frame);
  free_frame(task_union_frame);

  // Use the scheduler interface to select a new process to be executed and make a context switch.
  sched_next_rr();
  while(1);
}

void sys_block()
{
  // Blocks the current process if there are no pending unblocks for it. 
  // Otherwise, it decreases the number of pending unblocks for the process and does not block it.
  struct task_struct *current_task = current();
  if (current_task->pending_unblocks > 0) current_task->pending_unblocks--;
  else {
    update_process_state_rr(current_task, &blocked);
    sched_next_rr();
  }
}


int sys_unblock(int pid)
{
  // Unblocks the process with the given PID. If there is no process with this PID or the process with this PID is not blocked, it returns an error.
  struct task_struct *current_task = current();
  struct list_head *pos;
  list_for_each(pos, &current_task->children) {
    struct task_struct *child = list_entry(pos, struct task_struct, sibling);
    if (child->PID == pid) {
      if (child->state != ST_BLOCKED) {
        child->pending_unblocks++;
        return 0;
      }
      else {
        update_process_state_rr(child, &readyqueue);
        return 0;
      }
    }
  }
  return -ESRCH;
}

int sys_getchar(void)
{
    char c;
    if (del_circular_buffer(&c, &keyboard_buffer) != 0) return -ENODATA;
    return (int)c;
}



