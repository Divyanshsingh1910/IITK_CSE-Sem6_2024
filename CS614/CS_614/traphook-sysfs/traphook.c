#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
extern int (*trap_fault_hook)(struct pt_regs *regs, unsigned long error_code, unsigned long trapnr);
static int enabled = 0;

int my_handler(struct pt_regs *regs, unsigned long error_code, unsigned long trapnr)
{
  struct task_struct *tsk = current;
  printk(KERN_INFO "Got trap/fault %ld for pid = %d @%lx\n", trapnr, tsk->pid, regs->ip);
  regs->ip += 2;
  return 1;
  
}
static ssize_t traphook_status(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", enabled);
}

static ssize_t traphook_set(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        int newval;
        int err = kstrtoint(buf, 10, &newval);
        if (err || newval < 0 || newval > 1 )
                return -EINVAL;

        if(enabled && !newval)
                 trap_fault_hook = NULL;
        else if(!enabled && newval)
                 trap_fault_hook = &my_handler;
        enabled = newval;
       
        return count;
}

static struct kobj_attribute traphook_attribute = __ATTR(traphook, 0644, traphook_status, traphook_set);
static struct attribute *traphook_attrs[] = {
        &traphook_attribute.attr,
        NULL,
};
static struct attribute_group traphook_attr_group = {
        .attrs = traphook_attrs,
        .name = "cs614hook",
};

int init_module(void)
{
        int ret = sysfs_create_group (kernel_kobj, &traphook_attr_group);
        if(unlikely(ret))
                printk(KERN_INFO "demo: can't create sysfs\n");

	printk(KERN_INFO "All set to play\n");
	return 0;
}

void cleanup_module(void)
{
        sysfs_remove_group (kernel_kobj, &traphook_attr_group);
        trap_fault_hook = NULL;
	printk(KERN_INFO "Removed the trap hook handler\n");
}
MODULE_LICENSE("GPL");
