//From https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf Chap-3

#include <stdio.h>
#include <string.h>
#include "mpi.h"

int main( int argc, char *argv[])
{
  char message[20], recvmessage[100];
  int myrank;
  MPI_Status status;
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  if (myrank == 0)    /* code for process 0 */
  {
    strcpy(message,"Jai Shree Ram");
    MPI_Send(message, strlen(message)+1, MPI_CHAR, 1, 99, MPI_COMM_WORLD);
    printf("process %d sent : %s\n", myrank, message);
  }
  }
  else if (myrank == 1)  /* code for process 1 */
  {
    MPI_Recv(recvmessage, 10, MPI_CHAR, 0, 99, MPI_COMM_WORLD, &status);
    printf("process %d received : %s\n", myrank, message);
  }
  MPI_Finalize();
  return 0;
}
