#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>

int init_module(void)
{
        char buf[16];
        get_task_comm(buf, current); 
	printk(KERN_INFO "Hello kernel from cs614, this is the addition of a new module.\n The informations are: pid = %d name = %s\n", current->pid, buf);
	return 0;
}

void cleanup_module(void)
{	
	char buf[16];
	get_task_comm(buf, current); 
	printk(KERN_INFO "Goodbye kernel, the module has been detached.\n The informations are: pid = %d name = %s\n", current->pid, buf);
}
MODULE_LICENSE("GPL");
/* additional comments */
