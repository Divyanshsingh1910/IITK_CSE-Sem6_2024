#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/unistd.h>

void* thread_function(void *arg)
{
	int i = 0;
    while(i < 20){
        printf("Thread %d is running: %d\n",gettid() , i++);
        sleep(1);
    }
}
void* thread_function2(void *arg)
{
	int i = 0;
    while(i < 4){
        printf("Thread %d is running: %d\n",gettid() , i++);
        sleep(1);
    }
    syscall(549);
}

int main()
{
	pthread_t tid;
    pthread_t tids[7];
	printf("Hello\n");
    for(int i=0;i<3;i++)
    {
   	    pthread_create(&tid, NULL, &thread_function, NULL);
        tids[i] = tid;
    }

    for(int i=0;i<7;i++)
        printf("parent running pid = %d\n", getpid());
    
    pthread_create(&tid, NULL, &thread_function2, NULL);
	sleep(5);
	syscall(548);

    for(int i=0;i<3;i++)
    {
        pthread_join(tids[i], NULL);
        printf("parent running after syscall pid = %d\n", getpid());
    }    
    
   	// printf("Bye\n");
	pthread_join(tid, NULL);
    printf("exiting....\n");
    return 0;
}