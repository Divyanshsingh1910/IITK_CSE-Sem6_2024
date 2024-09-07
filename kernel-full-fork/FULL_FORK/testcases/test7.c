#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define NUM_THREADS 3
#define SLEEPTIME 5
#define ITERATE_NUM 5

long long fibonacci(int n) {
    if (n <= 1)
        return n;
    else
        return (fibonacci(n - 1) + fibonacci(n - 2));
}

void *thread_one(void *arg) {
    int thread_id = *(int *)arg;
    printf("Thread-1 started (thread ID: %d)\n", thread_id);
	int pid = fork();
	if(!pid){
		printf("This is the child of thread-1\n");
	}
	else{
		wait(NULL);
		printf("This is the parent of thread-1\n");
	}	
	
    printf("Thread-1 exiting (thread ID: %d)\n", thread_id);
    return NULL;
}

void *thread_two(void *arg) {
    int thread_id = *(int *)arg;
    printf("Thread-2 started (thread ID: %d)\n", thread_id);
	//exit(0);
    printf("Thread-2 exiting (thread ID: %d)\n", thread_id);
    return NULL;
}

void *thread_three(void *arg) {
    int thread_id = *(int *)arg;
    printf("Thread-3 started (thread ID: %d)\n", thread_id);
	int n = 41;
	long long result = fibonacci(n);
    printf("Thread-3 exiting (thread ID: %d)\n", thread_id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS] = {0, 1, 2};

    printf("Thread creation started\n");
    
    pthread_create(&threads[0], NULL, thread_one, (void *)&thread_ids[0]);

    pthread_create(&threads[1], NULL, thread_two, (void *)&thread_ids[1]);
	
	syscall(548);

    pthread_create(&threads[2], NULL, thread_three, (void *)&thread_ids[2]);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
