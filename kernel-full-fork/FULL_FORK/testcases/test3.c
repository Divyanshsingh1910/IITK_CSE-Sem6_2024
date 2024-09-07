#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 3
#define SLEEPTIME 5
#define ITERATE_NUM 5

void *sleeper_thread(void *arg) {
    int thread_id = *(int *)arg;
    printf("Sleeper thread started (thread ID: %d)\n", thread_id);
    sleep(SLEEPTIME); // Sleep for 5 seconds
    printf("Sleeper thread exiting (thread ID: %d)\n", thread_id);
    return NULL;
}

void *compute_thread(void *arg) {
    int thread_id = *(int *)arg;
    printf("Compute thread started (thread ID: %d)\n", thread_id);
    // Perform a time-consuming computation
    double v1 = 67899.08683548469474947;
    double v2 = 783956239.7948734739837494;
    double v3;
    int sum = 0;
    for (int i = 0; i < 100000000; i++) {
        sum += i;
        v3 = v1 * v2 + i;
    }
    printf("Compute thread exiting (thread ID: %d, sum = %d)\n", thread_id, sum);
    return NULL;
}

void *simple_thread(void *arg) {
    int thread_id = *(int *)arg;
    printf("Simple thread started (thread ID: %d)\n", thread_id);
    // Perform some simple tasks
    for (int i = 0; i < ITERATE_NUM; i++) {
        printf("Simple thread iteration %d (thread ID: %d)\n", i, thread_id);
        sleep(1); // Sleep for 1 second
    }
    printf("Simple thread exiting (thread ID: %d)\n", thread_id);
    return NULL;
}

int main() {

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS] = {0, 1, 2};

    printf("Thread creation started\n");
   

    pthread_create(&threads[0], NULL, sleeper_thread, (void *)&thread_ids[0]);
    printf("Thread 1 created\n");

    pthread_create(&threads[1], NULL, compute_thread, (void *)&thread_ids[1]);
    printf("Thread 2 created\n");

	syscall(548);

    pthread_create(&threads[2], NULL, simple_thread, (void *)&thread_ids[2]);
    printf("Thread 3 created\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
	

    printf("All threads have finished their execution.\n");
    return 0;
}
