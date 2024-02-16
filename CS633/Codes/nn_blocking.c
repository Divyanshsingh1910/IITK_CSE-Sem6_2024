#include <stdio.h>
#include <mpi.h>

int main(int argc, char* argv[]){
	int P = int(argv[1][0]-'0');

	MPI_Init(&argc,&argv);

	char data[]="hello";

}
