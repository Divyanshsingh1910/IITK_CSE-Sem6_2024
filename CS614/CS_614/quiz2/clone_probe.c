#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched.h>
#include<linux/sched/task_stack.h>
#include <linux/kprobes.h>
#include<linux/binfmts.h>

static unsigned long fork_ret_addr;
static int tracked_pid;

static int __kprobes before(struct kprobe *p, struct pt_regs *regs)
{
  struct kernel_clone_args *arg  = (struct kernel_clone_args *)regs->di;
 
 // if(current->pid != tracked_pid) 
//	  return 0;

  printk(KERN_INFO "Clone called by %d fn %lx stack %lx\n", current->pid, (unsigned long)arg->fn, arg->stack);

  //TODO: Part 1A: Depending on flags, classify user space calls as fork, vfork or pthread
	
  //TODO: Part 1B: For pthread, provide evidence of how the thread executes a function on return

  printk(KERN_INFO"Next instruction execution address: %lx\n",regs->ip);
  return 0;	
}


static struct kprobe kp_clone = {
    .symbol_name   = "kernel_clone",
	.pre_handler = before,
	.post_handler = NULL,
};


static ssize_t read_pid(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", tracked_pid);
}

static ssize_t set_pid(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        int newval;
        int err = kstrtoint(buf, 10, &newval);
        if (err || newval < 0)
                return -EINVAL;
        tracked_pid = newval;
        return count;
}

static struct kobj_attribute forkpidhook_attribute = __ATTR(tracked_pid, 0644, read_pid, set_pid);

static ssize_t get_fork_faddr(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%lx\n", fork_ret_addr);
}

static ssize_t set_fork_faddr(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        unsigned long newval;
        int err = _kstrtoul(buf, 16, &newval);
        if (err || newval < 0)
                return -EINVAL;
        fork_ret_addr = newval;
	printk("fork ret addr set as %lx\n", fork_ret_addr);
        return count;
}
static struct kobj_attribute forkaddrhook_attribute = __ATTR(fork_faddr, 0644, get_fork_faddr, set_fork_faddr);

static struct attribute *forkhook_attrs[] = {
        &forkpidhook_attribute.attr,
        &forkaddrhook_attribute.attr,
        NULL,
};
static struct attribute_group traphook_attr_group = {
        .attrs = forkhook_attrs,
        .name = "cs614hook",
};

int init_module(void)
{
	int ret;
	printk(KERN_INFO "Setting the probe\n");
	ret = register_kprobe(&kp_clone);
        if (ret < 0) {
                printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
                return ret;
        }
        printk(KERN_INFO "Planted kprobe at %lx\n", (unsigned long)kp_clone.addr);
	
	ret = sysfs_create_group (kernel_kobj, &traphook_attr_group);
        if(unlikely(ret))
                printk(KERN_INFO "demo: can't create sysfs\n");
        return 0;
}

void cleanup_module(void)
{
        sysfs_remove_group (kernel_kobj, &traphook_attr_group);
        unregister_kprobe(&kp_clone);
	printk(KERN_INFO "Removed the probes\n");
}
MODULE_LICENSE("GPL");
