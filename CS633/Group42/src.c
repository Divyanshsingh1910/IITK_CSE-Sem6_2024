#include <stdio.h>
#include "mpi.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

int 	cols,		/* number of columns in the matrix */
		rows,		/* number of rows in the matrix */
		Px,			/* number of processes involved in decomposition along x-axis */
		Py,			/* number of processes involved in decomposition along y-axis */
		width,		/* halo region width = stencil/4 */
		myrank,		/* rank of the current process in MPI_COMM_WORLD */
		localrank;	/* rank of the current process in local communicator */

double	**data = NULL,	/* the matrix containing data */
		**temp = NULL, 	/* to temporarily store new values after computation */
		*gather_buf = NULL,
		*scatter_buf = NULL;

bool    has_left_neighbour = true,		/* whether there exists left neighbour */
		has_right_neighbour = true,		/* whether there exists right neighbour */
		has_top_neighbour = true,		/* whether there exists top neighbour */
		has_bottom_neighbour = true;	/* whether there exists bottom neighbour */

double	*from_left = NULL,		/* data received from left neighbour */
		*from_right = NULL,		/* data received from right neighbour */
		*from_top = NULL,		/* data received from top neighbour */
		*from_bottom = NULL;	/* data received from bottom neighbour */

double	*to_left = NULL,	/* data sent to left neighbour*/
		*to_right = NULL,	/* data sent to right neighbour*/
		*to_top = NULL,		/* data sent to top neighbour*/
		*to_bottom = NULL;	/* data sent to bottom neighbour*/

bool	leader = false;			// whether the current run has heirarchical communication
bool	is_leader = false;	// is current rank leader

MPI_Comm localcomm;	/* local communicator */

void fill_has_neighbours()
{
    if(myrank%Px == 0) /* 0, 3, 6, 9 */ {
        has_left_neighbour = false;
		is_leader = true;
	}
    
	if(myrank%Px == Px - 1) /* 2, 5, 8, 11 */
        has_right_neighbour = false;
    
	if(myrank/Px == 0) /* 0, 1, 2 */
        has_top_neighbour = false;
    
	if(myrank/Px == Py - 1) /* 9, 10, 11 */
        has_bottom_neighbour = false;
}

void communicate(){
	int position = 0;
	MPI_Status status;

	/* packing data to send to neighbours */
	if(has_left_neighbour){
		position = 0;
		for(int j = 0; j < width; j++)
			for(int i = 0; i < rows; i++)
				MPI_Pack(&data[i][j], 1, MPI_DOUBLE, to_left, rows*width * sizeof(double), &position, MPI_COMM_WORLD);
	}
	
	if(has_right_neighbour){
		position = 0;
		for(int j = cols - width; j < cols; j++)
			for(int i = 0; i < rows; i++)
				MPI_Pack(&data[i][j], 1, MPI_DOUBLE, to_right, rows*width * sizeof(double), &position, MPI_COMM_WORLD);
	}
	
	if(has_top_neighbour){
		position = 0;
		for(int i = 0; i < width; i++)
			for(int j = 0; j < cols; j++)
				MPI_Pack(&data[i][j], 1, MPI_DOUBLE, to_top, cols*width * sizeof(double), &position, MPI_COMM_WORLD);
	}

	if(has_bottom_neighbour){
		position = 0;
		for(int i = rows - width; i < rows; i++)
			for(int j = 0; j < cols; j++)
				MPI_Pack(&data[i][j], 1, MPI_DOUBLE, to_bottom, cols*width * sizeof(double), &position, MPI_COMM_WORLD);
	}

/* note: as each process has distinct neighbours, while sending and receiving messages, tag isn't necessary, that's why tag = 0 everywhere */

/* along_x_communication */

	/* every even-process sends to right neighbour && odd-process sends to left*/
	switch((myrank%Px) % 2){
		case(0):
			if(has_right_neighbour){
				/* Sending to right */
				MPI_Send(to_right, rows*width * sizeof(double), MPI_PACKED, myrank + 1, 0, MPI_COMM_WORLD);

				/* Receiving from right*/
				MPI_Recv(from_right, rows*width * sizeof(double), MPI_PACKED, myrank + 1, 0, MPI_COMM_WORLD, &status);
			}
			break;

		case(1):
			if(has_left_neighbour){
				/* Receiving from left */
				MPI_Recv(from_left, rows*width * sizeof(double), MPI_PACKED, myrank - 1, 0, MPI_COMM_WORLD, &status);

				/* Sending to left */
				MPI_Send(to_left, rows*width * sizeof(double), MPI_PACKED, myrank - 1, 0, MPI_COMM_WORLD);
			}
			break;
	}

	/* every even-process sends to left neighbour && odd-process sends to right */
	switch((myrank%Px) % 2){	
		case(0):
			if(has_left_neighbour){
				/* Sending to left */
				MPI_Send(to_left, rows*width * sizeof(double), MPI_PACKED, myrank - 1, 0, MPI_COMM_WORLD);

				/* Receiving from left*/
				MPI_Recv(from_left, rows*width * sizeof(double), MPI_PACKED, myrank - 1, 0, MPI_COMM_WORLD, &status);
			}
			break;

		case(1):
			if(has_right_neighbour){
				/* Receiving from right */
				MPI_Recv(from_right, rows*width * sizeof(double), MPI_PACKED, myrank + 1, 0, MPI_COMM_WORLD, &status);

				/* Sending to right */
				MPI_Send(to_right, rows*width * sizeof(double), MPI_PACKED, myrank + 1, 0, MPI_COMM_WORLD);
			}
			break;
	}

/* along_y_communication */
	
	/* Here the parity is checked by
	 * division from Px
	 * So, 			0 , top most
	 *        	   /
	 * myrank/Px =
	 *			   \
	 *				Py-1, bottom most
	 */
	 
	/* every even-process sends to bottom && odd-process sends to top */
	if (!leader) {
		switch((myrank/Px) % 2){	
			case(0):
				if(has_bottom_neighbour){
					/* Sending to bottom */
					MPI_Send(to_bottom, cols*width * sizeof(double), MPI_PACKED, myrank + Px, 0, MPI_COMM_WORLD);

					/* Receiving from bottom */
					MPI_Recv(from_bottom, cols*width * sizeof(double), MPI_PACKED, myrank + Px, 0, MPI_COMM_WORLD, &status);
					// gather at leader, leader sends, leader recieves, scatter. Opposite order in case(1)
				}
				break;

			case(1):
				if(has_top_neighbour){
					/* Receiving from top */
					MPI_Recv(from_top, cols*width * sizeof(double), MPI_PACKED, myrank - Px, 0, MPI_COMM_WORLD, &status);

					/* Sending to top */
					MPI_Send(to_top, cols*width * sizeof(double), MPI_PACKED, myrank - Px, 0, MPI_COMM_WORLD);
				}
				break;
		}

		/* every even-process sends to top && odd-process sends to bottom*/
		switch((myrank/Px) % 2){
			case(0):
				if(has_top_neighbour){
					/* Sending to top */
					MPI_Send(to_top, cols*width * sizeof(double), MPI_PACKED, myrank - Px, 0, MPI_COMM_WORLD);

					/* Receiving from top */
					MPI_Recv(from_top, cols*width * sizeof(double), MPI_PACKED, myrank - Px, 0, MPI_COMM_WORLD, &status);
				}
				break;

			case(1):
				if(has_bottom_neighbour){
					/* Receiving from bottom */
					MPI_Recv(from_bottom, cols*width * sizeof(double), MPI_PACKED, myrank + Px, 0, MPI_COMM_WORLD, &status);

					/* Sending to bottom */
					MPI_Send(to_bottom, cols*width * sizeof(double), MPI_PACKED, myrank + Px, 0, MPI_COMM_WORLD);
				}
				break;
		
		}
	}
	else {
		switch ((myrank/Px)%2) {
			case(0):
				if (has_bottom_neighbour) {
					MPI_Gather(to_bottom, cols*width*sizeof(double), MPI_PACKED,
						   	gather_buf, cols*width*sizeof(double), MPI_PACKED,
						   	0, localcomm);
					if (is_leader){ // leader
						MPI_Send(gather_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank + Px, 0, MPI_COMM_WORLD);
						MPI_Recv(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank + Px, 0, MPI_COMM_WORLD, &status);
					}
					MPI_Scatter(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
							from_bottom, cols*width*sizeof(double), MPI_PACKED,
							0, localcomm);
				}
				break;

			case(1): 
				if (has_top_neighbour) {
					MPI_Gather(to_top, cols*width*sizeof(double), MPI_PACKED,
						   	gather_buf, cols*width*sizeof(double), MPI_PACKED,
						   	0, localcomm);
					if (is_leader) {
						MPI_Recv(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank - Px, 0, MPI_COMM_WORLD, &status
								);
						MPI_Send(gather_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank - Px, 0, MPI_COMM_WORLD);
					}
					MPI_Scatter(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
							from_top, cols*width*sizeof(double), MPI_PACKED,
							0, localcomm);
				}
				break;

		}
		switch ((myrank/Px)%2) {
			case (0):
				if (has_top_neighbour) {
					MPI_Gather(to_top, cols*width*sizeof(double), MPI_PACKED,
						   	gather_buf, cols*width*sizeof(double), MPI_PACKED,
						   	0, localcomm);
					if (is_leader) {
						MPI_Recv(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank - Px, 0, MPI_COMM_WORLD, &status
								);
						MPI_Send(gather_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank - Px, 0, MPI_COMM_WORLD);
					}
					MPI_Scatter(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
							from_top, cols*width*sizeof(double), MPI_PACKED,
							0, localcomm);
				}
				break;
					
			case (1):
				if (has_bottom_neighbour) {
					MPI_Gather(to_top, cols*width*sizeof(double), MPI_PACKED,
						   	gather_buf, cols*width*sizeof(double), MPI_PACKED,
						   	0, localcomm);
					if (is_leader) {
						MPI_Send(gather_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank + Px, 0, MPI_COMM_WORLD);
						MPI_Recv(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
								myrank + Px, 0, MPI_COMM_WORLD, &status
								);
					}
					MPI_Scatter(scatter_buf, cols*width*sizeof(double), MPI_PACKED,
							from_top, cols*width*sizeof(double), MPI_PACKED,
							0, localcomm);
				}
				break;
		}
	}
}

void swap(double ***a, double ***b)
{
	double **c = *a;
	*a = *b;
	*b = c;
}

double get_val(int i, int j, int *nneighbours)
{
	if (i < 0) {
		if (has_top_neighbour) {
			(*nneighbours) = (*nneighbours) + 1;
			return from_top [cols*(i+width) + j];
		}
		else
			return 0;
	} else if (i >= rows) {
		if (has_bottom_neighbour) {
			(*nneighbours) = (*nneighbours) + 1;
			return from_bottom[cols*(i - rows) + j];
		}
		else
			return 0;
	} else if (j < 0) {
        if (has_left_neighbour) {
			(*nneighbours) = (*nneighbours) + 1;
			return from_left[(j+width)*rows + i];
		}
		else
			return 0;
	} else if (j >= cols) {
        if (has_right_neighbour) {
			(*nneighbours) = (*nneighbours) + 1;
			return from_right[rows*(j-cols) + i];
		}
		else
			return 0;
	}

	(*nneighbours) = (*nneighbours) + 1;
	return data[i][j];
}

void compute(int i, int j)
{
    double sum_neighbours = 0;
	int nneighbours = 0;

   	for(int _ = 1; _ <= width; _++)
		sum_neighbours += get_val(i, j - _, &nneighbours);

   	for(int _ = 1; _ <= width; _++)
		sum_neighbours += get_val(i, j + _, &nneighbours);
   	
	for(int _ = 1; _ <= width; _++)
		sum_neighbours += get_val(i - _, j, &nneighbours);
   	
	for(int _ = 1; _ <= width; _++)
		sum_neighbours += get_val(i + _, j, &nneighbours);
	
	temp[i][j] = (data[i][j] + sum_neighbours)/(nneighbours + 1);
}

int main(int argc, char *argv[]) 
{
	int N,				/* number of data points per process */
		P,				/* total number of processes */
		num_time_steps,	/* number of steps */
		seed,
		stencil;
	
	if(argc != 5){
		printf("Error: wrong number of arguments provided\n");
		return 0;
	}

	/* command line arguments */
	Px = atoi(argv[1]),
	N = atoi(argv[2]),
	num_time_steps = atoi(argv[3]),
	seed = atoi(argv[4]),
	stencil = 5;
	
	/* initialize MPI */
	MPI_Init (&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	
	MPI_Comm_split(MPI_COMM_WORLD, myrank/Px, myrank, &localcomm);
	MPI_Comm_rank(localcomm, &localrank);

	Py = P/Px;
	rows = cols = sqrt(N);
	width = stencil/4;

	/* filling neighbours */
	fill_has_neighbours();
	
	/* allocating memory for the matrices and gather/scatter buffers */
	data = (double **)malloc(rows*sizeof(double*));
	temp = (double **)malloc(rows*sizeof(double*));
	
	gather_buf = (double *) malloc (Px * cols * width * sizeof(double));
	scatter_buf = (double *) malloc (Px * cols * width * sizeof(double));
	
	for(int i=0; i<rows; i++){
		data[i] = (double *)malloc(cols*sizeof(double));
		temp[i] = (double *)malloc(cols*sizeof(double));
	}

	/* allocating memory for data going to send to/receive from neighbours */
	if(has_left_neighbour){
		from_left	= (double *)malloc(rows*width * sizeof(double));
		to_left		= (double *)malloc(rows*width * sizeof(double));
	}
	
	if(has_right_neighbour){
		from_right	= (double *)malloc(rows*width * sizeof(double));
		to_right	= (double *)malloc(rows*width * sizeof(double));
	}
	
	if(has_top_neighbour){
		from_top	= (double *)malloc(cols*width * sizeof(double));
		to_top		= (double *)malloc(cols*width * sizeof(double));
	}

	if(has_bottom_neighbour){
		from_bottom	= (double *)malloc(cols*width * sizeof(double));
		to_bottom	= (double *)malloc(cols*width * sizeof(double));
	}
	
	/* initializing the matrix with random values */
	for(int i=0; i<rows; i++){
		for(int j=0; j<cols; j++){
			srand(seed*(myrank + 10));
			data[i][j] = abs(rand() + (i*rand() + j*myrank));
		}
	}

/* stencil communication + computation*/
	
	double start_time, end_time;
	
	/* with leader */
	leader = 1;
	start_time = MPI_Wtime();
	for(int steps = 0; steps < num_time_steps; steps++){
		communicate();
		for(int i=0; i<rows; i++){
			for(int j=0; j<cols; j++){
				compute(i, j);
			}
		}
		/* as temp contains new values, we need to swap data and temp */
		swap(&data, &temp); 
	}
	end_time = MPI_Wtime();

	end_time -= start_time;
	
	double max_time;
	MPI_Reduce (&end_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
	if (myrank == 0)
		printf ("Time with leader = %lf\n", max_time);

	/* without leader */
	leader = 0;
	start_time = MPI_Wtime();
	for(int steps = 0; steps < num_time_steps; steps++){
		communicate();
		for(int i=0; i<rows; i++){
			for(int j=0; j<cols; j++){
				compute(i, j);
			}
		}
		/* as temp contains new values, we need to swap data and temp */
		swap(&data, &temp); 
	}
	end_time = MPI_Wtime();

	end_time -= start_time;
	
	max_time;
	MPI_Reduce (&end_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
	if (myrank == 0){
		printf ("Time without leader = %lf\n", max_time);
		printf ("Data = %lf\n", data[0][0]);
	}

	/* done with MPI */
  	MPI_Finalize();
	return 0;
}
