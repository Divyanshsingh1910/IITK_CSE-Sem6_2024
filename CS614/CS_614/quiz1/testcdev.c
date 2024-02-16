#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include <sys/unistd.h>

static int do_read(long fd, char *rptr)
{
  long retval; 	
   retval = read(fd, rptr, 4096);
   if(retval == 4096){
        printf("Read successful\n");
	return 0;
   }else{
	perror("read");
   }
   return -1;
}
static int do_write_read(int fd, char *wptr, char *rptr)
{
  if(write(fd, wptr, strlen(wptr)) < 0){  
       perror("write");
       return -1;
   }
  printf("Write successful. I am going to read now!\n");
  return do_read(fd, rptr);  
}

void only_one_read(char *buf, long retval)
{
  if(retval == 4096){	
        printf("%s: Read: %s\n", __func__, buf); 
  }else{
	  perror("read");
  }
  exit(0); 	
}
int main()
{
   void (*impose)(char*, long) = NULL;
   
   /*Changes here*/
   printf("Address of only_one_read function: %p\n",&impose);

   char rbuf[4096]= {0};
   printf("Address of rbuf function: %p\n",rbuf);
   printf("Difference in address: %p\n\n",(unsigned long)rbuf-(unsigned long)&impose);
   int fd = open("/dev/democdev",O_RDWR);
   if(fd < 0){
       perror("open");
       exit(-1);
   }
  do_write_read(fd, "Hello! I am CS614", rbuf);
  printf("%s: Read: %s\n", __func__, rbuf);
  close(fd);
  return 0;
}
