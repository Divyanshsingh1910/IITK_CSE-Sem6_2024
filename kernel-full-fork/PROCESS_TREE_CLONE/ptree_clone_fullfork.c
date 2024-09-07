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
#include <linux/timekeeping.h>


//#include "queue.h"
#include "list.h"
#include <linux/rcupdate.h>
#include <linux/rwlock.h>
#include <linux/list.h>

/************************************************************/
/*fullfork*/
extern void add_to_pid_list(pid_t pid);
extern struct task_struct* (*ff_parent_changer)(struct task_struct* tsk);
extern bool (*ff_group_leader)(struct task_struct* task);
extern bool (*ff_eligible_pid)(struct task_struct* task);
//extern void (*syscall_hook)(struct pt_regs *regs, int sysnr);
extern int (*fullfork_wait)(pid_t pid, int*stat);

extern void  make_them_sleep(struct task_struct* temp);

extern struct list_head *pointer_to_pid_list_head;

volatile bool everything_done = false;
extern void (*pull_back)(void);
extern pid_t kernel_clone(struct kernel_clone_args *kargs);
extern pid_t my_kernel_clone(struct kernel_clone_args *kargs);
// extern void wake_up_new_task(struct task_struct *tsk);

extern int tracked_pid ;
extern int tracked_tgid;
static bool clone = false;
int error_check = 0;
extern struct task_struct *manager; 
unsigned long long times[6];

extern struct task_struct* p_prime;

extern wait_queue_head_t wq1; // wait queue for M
static wait_queue_head_t wq2;

bool group_leader(struct task_struct* task)
{
	if(task->tgid != tracked_tgid) return false;
	if(task->exit_signal >= 0) return true;
	return false;
}

bool is_eligible_pid(struct task_struct* task)
{
	if(task == manager){
		return true;
	}

	if(task->tgid != tracked_tgid) return false;
	return true;
}

struct task_struct* parent_changer(struct task_struct* tsk)
{
	if(current->tgid == tracked_tgid)
	{
		//return tsk->group_leader;
		return manager;
	}
	return tsk->group_leader->real_parent;
}

int leader_clone(void)
{
	struct kernel_clone_args args = {
		.exit_signal = SIGCHLD,
	};
	int child_pid;

	child_pid = kernel_clone(&args);
	if(child_pid < 0){
		printk(KERN_ERR "Kernel Clone: error on first fork %d\n", child_pid);
		return child_pid;
	}

	times[3] = ktime_get_real_ns();
	//return error_check;
	return child_pid;
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
					// wake_up_new_task(child);
				}

				temp = next_thread(temp);
				temp_regs = task_pt_regs(temp);
			}

done:

			tracked_pid = -1;
			tracked_tgid = -1;
			wake_up(&wq1);
			clone = false;
			wait_event(wq2, everything_done);
		}
	}
}

//void make_them_sleep(struct task_struct *top)
//{
//	struct task_struct *leader, *parent, *child;
//
//	//rcu_read_lock();
//	leader = top = top->group_leader;
//down:
//	for_each_thread(leader, parent) {
//		list_for_each_entry_rcu(child, &parent->children, sibling) {
//			leader = child;
//			goto down;
//up:
//			;
//		}
//
//		printk(KERN_INFO "PID_FOUND = %d", parent->pid);
//		pid_to_fork = parent->pid;
//
//		if (thread_group_leader(parent))
//		{
//			tracked_pid = parent->pid;
//			tracked_tgid = parent->tgid;
//			temp = next_thread(parent);
//
//			if (temp == parent)
//				goto single_process;
//
//			/* check if any of the threads is alive */
//			while(temp->flags & PF_EXITING)
//			{
//				temp = next_thread(temp);
//
//				/*  none of the threads is alive */
//				if (temp == parent)
//					goto single_process;
//			}
//
//			ff_parent_changer = &parent_changer;
//			ff_group_leader = &group_leader;
//			ff_eligible_pid = &is_eligible_pid;
//
//			err = kill_pid(temp->thread_pid, SIGSTOP, 1);
//			if(err)
//			{
//				printk(KERN_ERR "ERR: kill FAILED = %d", err);
//				return err;
//			}
//
//			err = fullfork_wait(temp->pid, &status);
//			if(err < 0)
//			{
//				printk(KERN_INFO "WAIT Return_val: FullFork Wait FAILED = %d", err);
//				err = kill_pid((current->thread_pid), SIGCONT, 0);
//				return err;
//			}
//			task_clear_jobctl_pending(current, JOBCTL_STOP_PENDING);
//
//single_process:
//			set_tsk_need_resched(parent);
//			kick_process(parent);
//
//			printk(KERN_INFO "Kick process called for %d", parent->pid);
//			//wait_event_interruptible(wait_for_fork, pid_to_fork == -1);
//			wait_event_interruptible(wq1, tracked_pid == -1);
//
//
//			times[1] = ktime_get_real_ns();
//
//
//			ff_eligible_pid = NULL;
//			ff_parent_changer = NULL;
//			ff_group_leader = NULL;
//
//			times[2] = ktime_get_real_ns();
//
//
//			err = kill_pid((current->thread_pid), SIGCONT, 0);
//			if(err < 0)
//			{
//				printk(KERN_ERR "ERR: signal sending SIGCONT failed = %d", err);
//				return -1;
//			}
//
//			tracked_pid = -1;
//			tracked_tgid = -1;
//
//	
//		}
//	}
//
//	if (leader != top) {
//		child = leader;
//		parent = child->real_parent;
//		leader = parent->group_leader;
//		goto up;
//	}
//	//rcu_read_unlock();
//}
//EXPORT_SYMBOL(pull_them_in);

int my_fullfork_handler(void)
{
	//struct pt_regs *regs;
	/*Stop all threads using signal*/
	int err;
	int ret_val;

	//regs = current->regs; //bingo


	// 	tracked_pid = current->pid;
	// 	tracked_tgid = current->tgid;
	// 	temp = next_thread(current);
	// 
	// 	if (temp == current)
	// 		goto clone;
	// 
	// 	/* check if any of the threads is alive */
	// 	while(temp->flags & PF_EXITING)
	// 	{
	// 		temp = next_thread(temp);
	// 
	// 		/*  none of the threads is alive */
	// 		if (temp == current)
	// 			goto clone;
	// 	}
	// 
	// 	ff_parent_changer = &parent_changer;
	// 	ff_group_leader = &group_leader;
	// 	ff_eligible_pid = &is_eligible_pid;
	// 
	// 	err = kill_pid(temp->thread_pid, SIGSTOP, 1);
	// 	if(err)
	// 	{
	// 		printk(KERN_ERR "ERR: kill FAILED = %d", err);
	// 		return err;
	// 	}

	// 	times[1] = ktime_get_real_ns();
	// 
	// 	err = fullfork_wait(temp->pid, &status);
	// 	if(err < 0)
	// 	{
	// 		printk(KERN_INFO "WAIT Return_val: FullFork Wait FAILED = %d", err);
	// 		err = kill_pid((current->thread_pid), SIGCONT, 0);
	// 		return err;
	// 	}
	// 	task_clear_jobctl_pending(current, JOBCTL_STOP_PENDING);
	// 
	// 	ff_eligible_pid = NULL;
	// 	ff_parent_changer = NULL;
	// 	ff_group_leader = NULL;
	// 
	// 	times[2] = ktime_get_real_ns();

	ret_val = leader_clone();
	if(ret_val < 0)
	{
		printk(KERN_ERR "ERR: leader clone failed = %d", ret_val);
	}


	err = kill_pid((current->thread_pid), SIGCONT, 0);
	if(err < 0)
	{
		printk(KERN_ERR "ERR: signal sending SIGCONT failed = %d", err);
		return -1;
	}

	tracked_pid = -1;
	tracked_tgid = -1;



	return ret_val;
}

/* ptree_clone*/
extern pid_t my_kernel_clone(struct kernel_clone_args *args);
// extern void wake_up_new_task(struct task_struct *p);
extern void (*syscall_hook)(struct pt_regs *regs, int sysnr);
extern struct task_struct *pid_task(struct pid* pid, enum pid_type type);
extern void reparent_backup(struct task_struct *p, struct task_struct *new_parent);
extern void (*ptree_clone_hook)(int pid);
//extern void pull_them_in(struct task_struct *top);

//extern int my_fullfork_handler(void);

extern volatile int pid_to_fork;
extern wait_queue_head_t wait_for_fork;
//struct queue *q;
struct queue *list_of_pids;


void my_ptree_clone_handler(int pid)
{
	struct task_struct* current_of_pid;

	init_waitqueue_head(&wq1); // initializing wait queue for M
	init_waitqueue_head(&wq2); // initializing wait queue for M
	manager = current;

	/* Fetching the task struct from the passed pid */
	printk("my_ptree_clone got pid[%d]",pid);	
	current_of_pid = pid_task(find_get_pid(pid),PIDTYPE_PID);

	if(!current_of_pid){
		printk(KERN_INFO "Invalid pid passed to ptree-clone manager :(");
		return ;
	}

	//q = new_queue();
	//list_of_pids = new_queue();
	//	store_pid_list(current);
	/* At this point, we have stored all the pids of the
	 * process tree,
	 * Note: It might be possible that any process is dead during 
	 * execution kick_process?
	 */


	//printk(KERN_INFO "Sbki pid store ho chuki hai");
	//print_pid_list(list_of_pids);
	init_waitqueue_head(&wait_for_fork); // initializing wait queue for M
										 //printk(KERN_INFO "PULL THEM IN ME");
										 //pull_them_in(current, visitor);
	//pull_them_in(current_of_pid);
	make_them_sleep(current_of_pid);
	printk(KERN_INFO "Done ho gya bhai Saara kaam toh\n");
}





//pid_t fullfork(void)
//{
//        struct kernel_clone_args args = {
//                .exit_signal = SIGCHLD,
//        };
//        return my_kernel_clone(&args);
//}

static int __kprobes before_schedule(struct kprobe *p, struct pt_regs *regs)
{
	
	if(current->pid == pid_to_fork){
		pid_t clone_pid;
	

		printk(KERN_INFO "Process[%d] in Schedule in probe\n", current->pid);
		//clone_pid = my_fullfork_handler();
		clone_pid = leader_clone();
		if (clone_pid < 0)
		{
			printk(KERN_INFO "FULLFORK FAILED AT SCHED PROBE");
			return clone_pid;
		}
		printk(KERN_INFO "pid[%d] is the fork of pid[%d]\n",clone_pid,pid_to_fork);

		add_to_pid_list(clone_pid);
		p_prime = pid_task(find_get_pid(clone_pid),PIDTYPE_PID);
		//add_to_queue(q, clone_pid);
		pid_to_fork = -1;
		//done = true;
		//wake_up_interruptible(&wait_for_fork);
	}

	return 0;
}

static struct kprobe kp_schedule = {
	.symbol_name = "schedule",
	.pre_handler = before_schedule,
	.post_handler = NULL,
};

void my_syscall_handler(struct pt_regs *regs, int sysnr)
{
	if(sysnr == 548){
		everything_done = true;
		wake_up(&wq2);
	}
}

int init_module(void)
{
	int ret_val;
	printk(KERN_INFO "Setting kprobe:");
	ret_val = register_kprobe(&kp_schedule);
	if(ret_val < 0){
		printk(KERN_INFO "schedule kprobe: failed and returned = %d",ret_val);
		return ret_val;
	}

	printk(KERN_INFO "Planted krprobe for schedule at %lx",(unsigned long)kp_schedule.addr);

	syscall_hook = &my_syscall_handler;
	ptree_clone_hook = &my_ptree_clone_handler;
	//get_snapped = &my_get_snapped;

	/* fullfork */

	pull_back = &my_pull_back;

	printk("MODULE INSTALLED :-)");
	return 0;
}

void cleanup_module(void)
{		/*fullfork*/

	pull_back = NULL;
	syscall_hook = NULL;
	ff_parent_changer = NULL;
	ff_group_leader = NULL;
	ff_eligible_pid = NULL;
	tracked_pid = -1;
	tracked_tgid = -1;
	manager = NULL;	

	/*ptree_clone*/
	unregister_kprobe(&kp_schedule);
	//syscall_hook = NULL;
	ptree_clone_hook = NULL;
	//get_snapped = NULL;
	printk(KERN_INFO "Goodbye kernel\n");
}

MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
