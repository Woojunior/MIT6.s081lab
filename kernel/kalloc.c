// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kinit()
{ char lockname[8];//定义字符串保存锁的名字
  for(int i=0; i<NCPU ;i++){
    snprintf(lockname,sizeof(lockname),"kmem%i",i);
    initlock(&kmem[i].lock, lockname);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();  //关闭中断
  int cpu_Id=cpuid();//获取当前cpu的id
  acquire(&kmem[cpu_Id].lock);
  r->next = kmem[cpu_Id].freelist;
  kmem[cpu_Id].freelist = r;
  release(&kmem[cpu_Id].lock);
  pop_off();  //开启中断


}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  //如果有当前空闲链表有空闲，则使用当前链表
  //如果当前链表没有空闲，则去其他CPU窃取空闲链表
  push_off();  //关闭中断
  int cpu_Id=cpuid();//获取当前cpu的id
  acquire(&kmem[cpu_Id].lock);
  r = kmem[cpu_Id].freelist;
  if(r)
    //当前空闲列表不为空
    kmem[cpu_Id].freelist = r->next;
  
  else{
    //当前空闲列表为空
    for(int anotherId=0; anotherId<NCPU ; anotherId++){
       if(anotherId==cpu_Id){
        continue;//如果为当前CPU，则循环继续
       }
       acquire(&kmem[anotherId].lock);
       r = kmem[anotherId].freelist;
       if(r){
          kmem[anotherId].freelist = r->next;
          release(&kmem[anotherId].lock);
          break;
       }
        release(&kmem[anotherId].lock);//切换的CPU没有空闲链表，释放所在CPU的锁
    }
  }
  release(&kmem[cpu_Id].lock);
  pop_off();  //开启中断

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
