#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include <sys/unistd.h>

int main()
{
   char rbuf[4096]= {0};

   int fd = open("/dev/democdev",O_RDWR);

  //  return 0;

   if(fd < 0){
       perror("open");
       exit(-1);
   }

  
  if(write(fd, "Hello CS614!", 12) < 0){  
       perror("write");
       exit(-1);
   }

  
 if(read(fd, rbuf, 4096) < 0){
    perror("read");
    exit(-1);
   }
  
  printf("Read: %s\n", rbuf);
  close(fd);
  return 0;
}
