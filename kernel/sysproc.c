#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 va;
  int len;
  uint64 user_mask_ptr;

  // Retrieve system call arguments from user space
  argaddr(0, &va);
  argint(1, &len);
  argaddr(2, &user_mask_ptr);

  // Limit the number of pages to scan
  if(len > 64) 
    return -1;

  struct proc *p = myproc();
  unsigned int abits = 0;

  // Iterate through each page to check the access bit
  for(int i = 0; i < len; i++){
    uint64 page_va = va + i * PGSIZE;
    
    // Find the Page Table Entry (PTE) for the given virtual address
    pte_t *pte = walk(p->pagetable, page_va, 0);
    
    // Check if the PTE exists, is valid (PTE_V), and has been accessed (PTE_A)
    if(pte != 0 && (*pte & PTE_V) && (*pte & PTE_A)){
      // Set the corresponding bit in the bitmask
      abits |= (1 << i);   
      
      // Clear the Access bit (PTE_A) as required to track future accesses
      *pte &= ~PTE_A;      
    }
  }

  // Copy the resulting bitmask back to user space memory
  if(copyout(p->pagetable, user_mask_ptr, (char *)&abits, sizeof(abits)) < 0)
    return -1;

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
