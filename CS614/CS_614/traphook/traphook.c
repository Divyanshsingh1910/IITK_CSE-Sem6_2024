#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
extern int (*trap_fault_hook)(struct pt_regs *regs, unsigned long error_code, unsigned long trapnr);
extern void (*syscall_hook)(struct pt_regs *regs, int sysnr);

int my_trap_handler(struct pt_regs *regs, unsigned long error_code, unsigned long trapnr)
{
  unsigned long sp;	
  struct task_struct *tsk = current;
  printk(KERN_INFO "Got trap/fault %ld for pid = %d RIP#%lx RSP#%lx RBP#%lx\n", trapnr, tsk->pid, regs->ip, regs->sp, regs->bp);
  asm volatile("mov %%rsp, %0;"
               :"=r"(sp)
               :
               :"memory"
	      );
  printk(KERN_INFO "Kernel RSP#%lx regs#%lx handler#%lx\n",sp, (unsigned long)regs, (unsigned long)trap_fault_hook);
  regs->ip += 2;
	return 1;
  
}
void my_syscall_handler(struct pt_regs *regs, int sysnr)
{
   unsigned long sp;	
   struct task_struct *tsk = current;
   if(!memcmp(current->comm, "a.out", 5) && sysnr == 1){
            printk(KERN_INFO "System call for pid = %d RIP#%lx RSP#%lx RBP#%lx\n", tsk->pid, regs->ip, regs->sp, regs->bp);
            asm volatile("mov %%rsp, %0;"
               :"=r"(sp)
               :
               :"memory"
	      );
            printk(KERN_INFO "Kernel RSP#%lx regs#%lx handler#%lx\n",sp, (unsigned long)regs, (unsigned long)syscall_hook);
        }
   return;
}

int init_module(void)
{
	printk(KERN_INFO "Setting the trap hook handler\n");
        trap_fault_hook = &my_trap_handler;
        syscall_hook = &my_syscall_handler;
	return 0;
}

void cleanup_module(void)
{
        trap_fault_hook = NULL;
        syscall_hook = NULL;
	printk(KERN_INFO "Removed the trap hook handler\n");
}

MODULE_LICENSE("GPL");
