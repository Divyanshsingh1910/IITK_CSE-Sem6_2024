#include <stdio.h>
#include "mpi.h"

int main( int argc, char *argv[]){

    // initialize MPI
    MPI_Init(&argc, &argv);

    printf("Hello, world!\n");

    // done with MPI
    MPI_Finalize();
}

/*
Compile​

    mpicc -o program.x program.c​

​

Execute​

    mpirun -np 1 ./program.x (mpiexec -np 1 ./program.x)​
        {Runs 1 process on the launch/login node​}


    mpirun -np 6 ./program.x​
        {Runs 6 processes on the launch/login node}

*/