#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define NUM_THREADS 3
#define SLEEPTIME 5
#define ITERATE_NUM 5


void *thread_one(void *arg) {
    int thread_id = *(int *)arg;
    printf("Thread-1 started (thread ID: %d) and pid:%d\n", thread_id,gettid());
	int pid = fork();
	if(!pid){
		printf("This is the child of thread-1\n");
	}
	
	wait(NULL);
	printf("This is the parent of thread-1\n");
	
    printf("Thread-1 exiting (thread ID: %d)\n", thread_id);
    return NULL;
}

void *thread_two(void *arg) {
    int thread_id = *(int *)arg;
    printf("Thread-2 started (thread ID: %d) and pid:%d\n", thread_id,gettid());
	
	pthread_t threads[1];
	int thread_ids[NUM_THREADS] = {0,1,2};

	pthread_create(&threads[0],NULL,thread_one, (void *)&thread_ids[0]);

	pthread_join(threads[0],NULL);

    printf("Thread-2 exiting (thread ID: %d)\n", thread_id);
    return NULL;
}


int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS] = {0, 1, 2};

    printf("Thread creation started\n");
    
    pthread_create(&threads[0], NULL, thread_two, (void *)&thread_ids[0]);
	syscall(548);
    pthread_join(threads[0], NULL);

    	//sleep(5);

	printf("last mee ");

    return 0;
}
