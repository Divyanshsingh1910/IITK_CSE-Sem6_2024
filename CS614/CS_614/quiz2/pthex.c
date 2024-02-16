#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>

#define THREAD_TEST
//#define VFORK_TEST
//#define FORK_TEST
//#define FORK_TEST_CHFUNC
#ifdef FORK_TEST_CHFUNC
void child_func(void)
{
   	printf("%s: I am child pid = %d\n", __func__, getpid());
	_exit(0);
}
#endif

#ifdef THREAD_TEST
void* thfunc(void *arg)
{
     int ctr = 0;
     printf("Thread stack pointer = %p\n", &ctr);
     return NULL;
}
#endif

int set_tracked_pid(int pid)
{
	char buf[16];
	int fd = open("/sys/kernel/cs614hook/tracked_pid", O_RDWR);
	if(fd < 0){
		perror("open");
		return fd;
	}
        sprintf(buf, "%d", pid);
	if(write(fd, buf, 16) < 0){
		perror("open");
		return -1;
	}
	printf("Process %d is being tracked now\n", pid);
	return 0;
}

#ifdef FORK_TEST_CHFUNC
int set_fork_addr(unsigned long addr)
{
	char buf[16];
	int fd = open("/sys/kernel/cs614hook/fork_faddr", O_RDWR);
	if(fd < 0){
		perror("open");
		return fd;
	}
        sprintf(buf, "%lu", addr);
	if(write(fd, buf, 16) < 0){
		perror("open");
		return -1;
	}
	printf("Setting fork return as %lx\n", addr);
	return 0;
}
#endif

int main()
{
   int pid;
   pthread_t tid;

  assert(set_tracked_pid(getpid()) == 0);

#if defined(FORK_TEST_CHFUNC) || defined(FORK_TEST) 	

#ifdef FORK_TEST_CHFUNC
   assert(set_fork_addr((unsigned long)&child_func) == 0);
#endif
  
   pid = fork();
  assert(pid >= 0);
  if(!pid){
	  printf("I am fork child with pid = %d\n", getpid());
	  _exit(0);
  }
  printf("Done with fork\n");
#endif

#ifdef VFORK_TEST	
  pid = vfork();
  assert(pid >= 0);
  if(!pid){
	  printf("I am vfork child with pid = %d\n", getpid());
	  _exit(0);
  }
   printf("Done with vfork\n");
#endif

#ifdef THREAD_TEST	
  if(pthread_create(&tid, NULL, thfunc, NULL) != 0){
              perror("pthread_create");
              exit(-1);
  }
  printf("pid = %d Main stack pointer = %p\n", getpid(), &tid);
  pthread_join(tid, NULL);
  printf("Done with pthread\n");
#endif

  return 0;
}
