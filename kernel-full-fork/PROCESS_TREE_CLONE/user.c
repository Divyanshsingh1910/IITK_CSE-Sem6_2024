#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>


//int set_tracked_pid(int pid)
//{
//        syscall(549);
//        printf("-----------------------\n");
//        printf("Ideally kaam ho gya hoga\n");
//        return 0;
//}


int main()
{
   int pid;
   pthread_t tid;

   // This is root process
   //child1
   printf("Execution Started with pid[%d]\n",getpid());
   int ret = fork();
        if(!ret){
                //child1

                int ret2 = fork();
                if(!ret2){
                        //child1 ka child
                        printf("Process[%d] <---- child1_ka_child\n",getpid());
                        while(1);
                }
                else{

                        //child1
                        printf("Process[%d] <---- child1\n",getpid());
                        while(1);
                }
        }
        else{
                ret = fork();
                if(!ret){
                        //child2
                        int ret3 = fork();
                        if(!ret3){
                                //child2 ka child
                                printf("Process[%d] <---- child2_ka_child\n",getpid());
                                while(1);
                        }
                        else{

                                //child2
                                printf("Process[%d] <---- child2\n",getpid());
                                while(1);
                        }
                }
                else{
                        sleep(2);
                        //root parent
   //                assert(set_tracked_pid(getpid()) == 0);

                   printf("[Parent]My pid %d\n", getpid());
                   while(1);
                }
        }


  while(1);
  return 0;
}
