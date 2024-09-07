#include <linux/list.h>

struct pid_list
{
        pid_t pid;
        struct list_head list;
};

struct pid_list* new_pid_list(pid_t pid)
{
        struct pid_list *temp = kmalloc(sizeof(struct pid_list), GFP_KERNEL);
        temp->pid = pid;
        INIT_LIST_HEAD(&temp->list);
        return temp;
}

bool in_pid_list(struct list_head *pid_list_head, pid_t pid)
{
        struct pid_list *temp;
        list_for_each_entry(temp, pid_list_head, list){
                if(temp->pid == pid){
                        return true;
                }
        }
        return false;
}
