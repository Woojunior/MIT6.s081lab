#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"


uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
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
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
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

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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

uint64
sys_trace(void)
{ 
  int n;
  if(argint(0, &n)<0)
    return -1;

  myproc()->arg=n;
  return 0;
}
// To collect the amount of free memory, add a function to kernel/kalloc.c
extern uint64 freebyte(void);
// To collect the number of processes, add a function to kernel/proc.c
extern uint64 num_proc(void);
//两个函数在这里声明，但是在另外两个文件里定义
//此时不能使用包含两个文件kernel/kalloc.c和 kernel/proc.c的方式，有同名的函数，会引起变量的冲突
//这样声明就是使编译器直到这个函数的存在，在链接的时候会找到这个函数的定义

uint64
sys_sysinfo(void)
{
  struct proc *p=myproc();//用来指向当前进程
  struct sysinfo _sysinfo;
  uint64 user_addr;//获取虚拟地址，即user给sysinfo这个系统调用的地址
  _sysinfo.freemem=freebyte();
  _sysinfo.nproc=num_proc();

  //获取用户态传来的地址
  if(argaddr(0,&user_addr)<0)
    return -1;//失败

  //将sysinfo从kernel mode传回给user mode给定的地址
  if(copyout(p->pagetable,user_addr,(char*) &_sysinfo,sizeof _sysinfo))
    return -1;//失败
  
  return 0;//成功
}