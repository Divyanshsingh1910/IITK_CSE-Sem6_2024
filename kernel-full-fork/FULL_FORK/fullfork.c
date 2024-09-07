#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kprobes.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/pid.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/entry-common.h>
#include <asm/processor.h>
#include <linux/sched/task_stack.h>
#include <linux/sched/jobctl.h>
#include <linux/thread_info.h>


extern struct task_struct* (*ff_parent_changer)(struct task_struct* tsk);
extern bool (*ff_group_leader)(struct task_struct* task);
extern bool (*ff_eligible_pid)(struct task_struct* task);
extern void (*syscall_hook)(struct pt_regs *regs, int sysnr);

extern void (*pull_back)(void);
extern pid_t kernel_clone(struct kernel_clone_args *kargs);
extern pid_t my_kernel_clone(struct kernel_clone_args *kargs);
extern void wake_up_new_task(struct task_struct *tsk);

int tracked_pid = -1;
int tracked_tgid = 0;
static bool clone = false;
int error_check = 0;

static wait_queue_head_t wq1; // wait queue for M

bool group_leader(struct task_struct* task)
{
	if(task->tgid != tracked_tgid) return false;
	if(task->exit_signal >= 0) return true;
	return false;
}

bool is_eligible_pid(struct task_struct* task)
{
	if(task->tgid != tracked_tgid) return false;
	return true;
}

struct task_struct* parent_changer(struct task_struct* tsk)
{
	if(current->tgid == tracked_tgid)
	{
		return tsk->group_leader;
	}
	return tsk->group_leader->real_parent;
}

int leader_clone(struct pt_regs *regs)
{
	struct kernel_clone_args args = {
		.exit_signal = SIGCHLD,
	};
	int child_pid;
	
	init_waitqueue_head(&wq1); // initializing wait queue for M
	child_pid = kernel_clone(&args);
	if(child_pid < 0){
		printk(KERN_ERR "Kernel Clone: error on first fork %d\n", child_pid);
		return child_pid;
	}

	wait_event(wq1, tracked_pid == -1);
	return error_check;
}

void my_pull_back(void)
{
	pid_t ppid = task_ppid_nr(current);	
	if(ppid == tracked_pid){

		if(!clone){

			struct task_struct *parent_task = current->real_parent, *child;
			int clone_pid;
			struct task_struct *temp;
			struct pt_regs *temp_regs, *my_regs;
		
			clone = true;    // any child formed by forking the task struct of this process is clone
				
			temp = next_thread(parent_task);		
			temp_regs = task_pt_regs(temp);
			while(temp != parent_task){

				/* clone only alive threads */
				if(!(temp->flags & PF_EXITING))
				{
					struct kernel_clone_args args = {
						.flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND
								| CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS
								| CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | 0,
						.child_tid = temp->clear_child_tid,
						.parent_tid = temp->clear_child_tid,
						.exit_signal = 0,
						.stack = temp->mm->start_stack,
						.stack_size = 0x7fff00, // Warning: hard coded
						.tls = temp->thread.fsbase,
					};
					
					clone_pid = my_kernel_clone(&args);
					if(clone_pid < 0){
						printk(KERN_INFO "Kernel Clone: error %d on kernel_clone\n", clone_pid);
						error_check = -1;
						goto done;
					};

					child = pid_task(find_vpid(clone_pid), PIDTYPE_PID);
					my_regs = task_pt_regs(child);
					*my_regs = *temp_regs;
					wake_up_new_task(child);
				}

				temp = next_thread(temp);
				temp_regs = task_pt_regs(temp);
			}
			
			done:
				tracked_pid = -1;
				tracked_tgid = -1;
				wake_up(&wq1);
				clone = false;
		}
  	}
}

void my_syscall_handler(struct pt_regs *regs, int sysnr)
{
	if (sysnr == 548)
	{

		/*Stop all threads using signal*/
		int err; 
		int status;
		struct task_struct *temp;	
		
		tracked_pid = current->pid;
		tracked_tgid = current->tgid;
		temp = next_thread(current);

		if (temp == current)
			goto clone;

		/* check if any of the threads is alive */
		while(temp->flags & PF_EXITING)
		{
			temp = next_thread(temp);

			/*  none of the threads is alive */
			if (temp == current) 
				goto clone;
		}

		ff_parent_changer = &parent_changer;
		ff_group_leader = &group_leader;
		ff_eligible_pid = &is_eligible_pid;

		err = kill_pid(temp->thread_pid, SIGSTOP, 1);
		if(err)
		{
			printk(KERN_ERR "ERR: kill FAILED = %d", err);
			return;
		}


		err = fullfork_wait(temp->pid, &status);
		if(err < 0)
		{
			printk(KERN_INFO "WAIT Return_val: FullFork Wait FAILED = %d", err);
			err = kill_pid((current->thread_pid), SIGCONT, 0);
			return;
		}
		task_clear_jobctl_pending(current, JOBCTL_STOP_PENDING);
		
		ff_eligible_pid = NULL;
		ff_parent_changer = NULL;
		ff_group_leader = NULL;


	clone:
		err = leader_clone(regs);
		if(err < 0)
        {
			printk(KERN_ERR "ERR: leader clone failed = %d", err);
        }

		err = kill_pid((current->thread_pid), SIGCONT, 0);
		if(err < 0)
		{
			printk(KERN_ERR "ERR: signal sending SIGCONT failed = %d", err);
			return;
		}
		tracked_pid = -1;
		tracked_tgid = -1;
	}
}

int init_module(void)
{
	pull_back = &my_pull_back;
	syscall_hook = &my_syscall_handler;
	printk(KERN_INFO "All set to play now\n");
	return 0;
}

void cleanup_module(void)
{
	pull_back = NULL;
	syscall_hook = NULL;
	ff_parent_changer = NULL;
	ff_group_leader = NULL;
	ff_eligible_pid = NULL;
	tracked_pid = -1;
	tracked_tgid = -1;
	printk(KERN_INFO "Removed the trap hook handler\n");
}

MODULE_LICENSE("GPL");
