struct qnode
{
	pid_t pid;
	struct qnode *next;
};

struct qnode* new_node(pid_t pid)
{
	struct qnode *temp = kmalloc(sizeof(struct qnode), GFP_KERNEL);
	temp->pid = pid;
	temp->next = NULL;
	return temp;
}

struct queue
{
	struct qnode *head;
	struct qnode *tail;
};

struct queue* new_queue(void)
{
	struct queue *temp = kmalloc(sizeof(struct queue), GFP_KERNEL);
	temp->head = NULL;
	temp->tail = NULL;
	return temp;
}

void add_to_queue(struct queue *q, pid_t pid)
{
	struct qnode *temp = new_node(pid);
	if(!q->head) // first element
		q->head = q->tail = temp;
	else{
		q->tail->next = temp;
		q->tail = temp;
	}
}

void remove_from_queue(struct queue *q)
{
	struct qnode *temp = q->head;
	if(q->head == q->tail) // only one element
		q->head = q->tail = NULL;
	else
		q->head = q->head->next;
	kfree(temp);
}

pid_t front_of_queue(struct queue *q)
{
	return q->head->pid;
}

bool pid_is_in_list(struct queue* q, pid_t pid)
{
	struct qnode* temp = q->head;
	while(temp != q->tail)
	{
		if(temp->pid == pid)
			return true;

		temp = temp->next;
	}
	if(temp && temp->pid == pid)
		return true; //checking at the tail

	return false;
}

void print_pid_list(struct queue* q)
{
	struct qnode* temp = q->head;
	int i = 1;

	while(temp != q->tail)
	{
		// printk(KERN_INFO "[%d] is the pid of {%d}",i,temp->pid);	
		i++;
		temp = temp->next;
	}
	i++;
	/* if(temp)
		printk(KERN_INFO "[%d] is the pid of {%d}",i,temp->pid);
		//checking at the tail
	*/
}
