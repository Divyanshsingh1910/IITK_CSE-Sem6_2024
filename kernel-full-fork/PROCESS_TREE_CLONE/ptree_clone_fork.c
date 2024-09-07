#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/rwlock.h>
#include <linux/rcupdate.h>
#include <linux/sched/task.h>
#include "list.h"

extern pid_t my_kernel_clone(struct kernel_clone_args *args);
extern void wake_up_new_task(struct task_struct *p);
extern void (*syscall_hook)(struct pt_regs *regs, int sysnr);
extern struct task_struct *pid_task(struct pid* pid, enum pid_type type);
extern void reparent(struct task_struct *p, struct task_struct *new_parent);

static volatile int target_pid = -1;
static wait_queue_head_t wq;
volatile bool done = false;
struct task_struct *p_prime;
LIST_HEAD(pid_list_head);

void print_children(struct task_struct *p)
{
        struct task_struct *c;
        printk(KERN_INFO "Printing chilren of pid: %d", p->pid);
        list_for_each_entry_rcu(c, &p->children, sibling){
              printk(KERN_INFO "-----> pid: %d", c->pid);
        }
}

void print_tree(struct task_struct *p)
{
        struct task_struct *c;
        printk(KERN_INFO "pid: %d", p->pid);
        list_for_each_entry(c, &p->children, sibling)
                print_tree(c);
}

void dfs(struct task_struct *p)
{
        /*
           c    ->      to iterate over children of p
           gc   ->      to iterate over grandchildren of p
        */
        struct task_struct *c, *gc;
        struct task_struct *temp1, *temp2;

        printk(KERN_INFO "DFS ME GHUSA %d", p->pid);
        list_for_each_entry(c, &p->children, sibling)
                dfs(c);

        printk(KERN_INFO "DFS SE NIKLA %d", p->pid);
        target_pid = p->pid;
        wait_event_interruptible(wq, done);
        printk(KERN_INFO "DONE = %d", done);
        done = false;

        /*
           p                             p'
           / \          ->              / \
           c_1   c_2             c_1'  c_2'

           Before reparenting
           ------------------
           Parent
           ------
           c_1' -> c_1
           c_2' -> c_2

           After reparenting
           -----------------
           Parent
           ------
           c_1' -> p'
           c_2' -> p'
           */

        list_for_each_entry_safe(c, temp1, &p->children, sibling){
                 if(!in_pid_list(&pid_list_head, c->pid)){
                         list_for_each_entry_safe(gc, temp2, &c->children, sibling){
                                 if(in_pid_list(&pid_list_head, gc->pid)){
                                        reparent(gc, p_prime);
                                 }
                         }
                 }
        }

        printk(KERN_INFO "Done with Children of P_prime[%d]: \n",p_prime->pid);
        target_pid = -1;
}

void my_syscall_handler(struct pt_regs *regs, int sysnr)
{
        if(sysnr == 548){
                struct task_struct* t;
                init_waitqueue_head(&wq); // initializing wait queue for M
                printk("Before cloning");
                print_tree(current);
                dfs(current);

                printk("After cloning");
                print_tree(p_prime);
                for_each_thread(p_prime, t)
                        wake_up_new_task(t);
                printk(KERN_INFO "Done ho gya bhai Saara kaam toh\n");
        }
}

pid_t fullfork(void)
{
        struct kernel_clone_args args = {
                .exit_signal = SIGCHLD,
        };
        return my_kernel_clone(&args);
}

static void __kprobes after_schedule(struct kprobe *p, struct pt_regs *regs, \
                long unsigned int temp)
{
        if(current->pid == target_pid){
                pid_t clone_pid;
                struct pid_list* new_pid;
                printk(KERN_INFO "Process[%d] in Schedule in probe\n", current->pid);

                clone_pid = fullfork();
                if (clone_pid < 0)
                {
                        printk(KERN_INFO "FULLFORK FAILED AT SCHED PROBE");
                        return;
                }

                printk(KERN_INFO "pid[%d] is the fork of pid[%d]\n",clone_pid,target_pid);

                new_pid = new_pid_list(clone_pid);
                list_add(&new_pid->list, &pid_list_head);

                p_prime = pid_task(find_get_pid(clone_pid),PIDTYPE_PID);

                target_pid = -1;
                done = true;
                wake_up_interruptible(&wq);
        }
}

static struct kprobe kp_schedule = {
        .symbol_name   = "schedule",
        .pre_handler = NULL,
        .post_handler = after_schedule,
};

int init_module(void)
{
        int ret_val;
        printk(KERN_INFO "Setting the probes\n");

        // kprobe after schedule
        ret_val = register_kprobe(&kp_schedule);
        if(ret_val < 0){
                printk(KERN_INFO "schedule kprobe: register_kprobe failed, returned %d\n", ret_val);
                return ret_val;
        }
        printk(KERN_INFO "Planted schedule kprobe at %lx\n", (unsigned long)kp_schedule.addr);
        syscall_hook = &my_syscall_handler;
        return 0;
}

void cleanup_module(void)
{
        unregister_kprobe(&kp_schedule);
        syscall_hook = NULL;
        printk(KERN_INFO "Goodbye kernel\n");
}

MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
