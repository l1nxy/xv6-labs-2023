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
// First, it takes the starting virtual address of the first user page to check. 
// Second, it takes the number of pages to check. 
// Finally, it takes a user address to a buffer to store the results into a bitmask (a datastructure that uses one bit per page and where the first page corresponds to the least significant bit).
int
sys_pgaccess(void)
{
  uint64 va;
  int pn;
  uint64 addr;
  argaddr(0,&va);
  argint(1,&pn);
  argaddr(2,&addr);

  if(pn > 32){
    return -1;
  }

  unsigned int result_bit = 0;
  pagetable_t pg = myproc()->pagetable;
  for(int i = 0; i < pn; ++i){
    pte_t *pte = walk(pg, va + i * PGSIZE, 0);
    if(pte == 0){
      continue;
    }
    if((*pte & PTE_A) != 0) {
      result_bit  |= 1 << i;
      *pte &= ~(PTE_A);
    }
  }

  copyout(pg, addr, (char *)&result_bit, sizeof(int));

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
