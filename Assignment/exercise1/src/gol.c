/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */

#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include "dynamics.h"
#include <omp.h>
#include <mpi.h>
#if !defined(_OPENMP)
#warning "Run "make openmp" to enable OpenMP."
#endif

int   action = 0;
int   k      = K_DFLT;
int   e      = STATIC;
int   n      = 10000;
int   s      = 1;
char *fname  = NULL;
int maxval = 255; //255 -> white, 0 -> black

//IDEA:
//To save memory, the storage can be reduced to one array plus two line buffers.
// One line buffer is used to calculate the successor state for a line, then the second 
// line buffer is used to calculate the successor state for the next line. The first buffer
// is then written to its line and freed to hold the successor state for the third line.

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    
    int size_of_cluster;
    int process_rank;
    int n_lines_per_process;
    void *map1 = NULL;
    void *map2 = NULL;
    unsigned char *map1_char = NULL;
    unsigned char *sub_map = NULL;
    char file[] = "images/blinker.pgm";
    char snapshot_name[100];

    MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    
    /* Things that only process 0 has to do:
        1. Parse command line arguments
        2. Read or generate matrix
        3. Calculate the lines per process*/
    if(process_rank == 0){
        #ifndef _OPENMP
            printf("\nExecuting without OPENMP in serial mode.\n");
        #endif
        command_line_parser(&action, &k, &e, &fname, &n, &s, argc, argv);
        if((action != RUN) && (action != INIT)){
	    printf("Rank %d: No action specified\n", process_rank);
            printf("Possible actions:\n"
                    "r - Run a world\n"
                    "i - Initialize a world\n"
                    "Evolution types:\n"
                    "e 0 - Ordered evolution\n"
                    "e 1 - Static evolution\n\n");
            exit(1);
        }

        n_lines_per_process = k/ size_of_cluster;
        /*NOTE: deal with the case where k%size_of_cluster !=0 */

        printf("Rank: %d \tNumber of lines per process: %d/%d=%d\n", process_rank, k, size_of_cluster, n_lines_per_process);

	map1 = (unsigned char*)malloc(k*k*sizeof(unsigned char));
	if (map1 == NULL)
	{
		printf("Error: Could not allocate memory for map1\n");
		exit(1);
	}

	set_up_map_variable(action, e, k, &map1, maxval, file);
        map1_char = (unsigned char*)map1;
	printf("Back to main\n");
	print_map(process_rank, k, k, map1);
	
        if (e == STATIC){
            static_set_up_other_map(map1, map2, k);
        } else if (e == ORDERED)
            init_to_zero(map1_char, k);
    }

    /*  Make every process wait for process 0 to perform the task above
	* communicate n_lines_per_process to every process */

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&n_lines_per_process, 1, MPI_INT, 0, MPI_COMM_WORLD);  
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);	
	MPI_Barrier(MPI_COMM_WORLD);

    #if defined(_OPENMP)
    #pragma omp parallel 
    {
        #pragma omp master 
        {
            int nthreads = omp_get_num_threads();
            printf("Going to use %d threads\n", nthreads );
        }
    }
    #endif
#ifdef PROFILING
        printf("***************************************\n");
        printf("In profiling mode, not generating images\n");
        printf("***************************************\n");
        double tstart  = CPU_TIME;
    #endif

    if (e == ORDERED){
        printf("\nOrdered evolution to be implemented with MPI\n");
        exit(1);
    } else if (e == STATIC){
	int rows_to_receive = nrows_given_process(process_rank, n_lines_per_process, k);
/*
	printf("process %d: start_row = %d*(%d+1)\n", process_rank, process_rank, n_lines_per_process);
	printf("process %d: end_row = %d + %d -1\n", process_rank, start_row, n_lines_per_process);
	printf("process %d: or end_row = %d -1 = %d\n", process_rank, k, end_row);
	printf("process %d: rows_to_receive = %d - %d +1\n", process_rank, end_row, start_row);
*/
	//Allocate space for local maps
	sub_map = (unsigned char*)calloc(rows_to_receive * k, sizeof(unsigned char));
/*	printf("Process %d allocated sub_map at %p\n", process_rank, sub_map); */ if (sub_map == NULL){
		printf("Error: Could not allocate memory for local maps\n");
		exit(1);
	}
	unsigned char* local_map = calloc((rows_to_receive+2)*k, sizeof(unsigned char));
	unsigned char* local_map2 = calloc((rows_to_receive+2)*k, sizeof(unsigned char));
	if(local_map == NULL || local_map2 == NULL){
		printf("Rank %d - Error in allocating local maps\n\n", process_rank);
		exit(1);
	}

/*        for(int i = 0; i < 5; i++){*/
        for(int i = 0; i < N_STEPS; i++){ 
		printf("Process %d, step %d\n", process_rank, i);	

		/* 1) Scattering the map 
		 * Not using MPI_Scatter because I want to specify the rows to send exactly. */
		if (process_rank ==0){
			for (int i = 0; i < size_of_cluster; i++){
				int rows_to_copy = nrows_given_process(i, n_lines_per_process, k);
				int start_row = process_rank * (n_lines_per_process);
				printf("DEBUG: process %d, rows_to_copy = %d\n", process_rank, rows_to_copy);
				MPI_Send(map1_char + start_row * k, rows_to_copy * k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
			}
		}
		MPI_Recv(sub_map, rows_to_receive * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


		for (int i=0; i<rows_to_receive; i++){
			for(int j=0; j<k; j++){
				int index = i*k+j;
				local_map[index+k] = sub_map[index];
			}
		}
		/* Writing first and last row */
		for (int i=0; i < k; i++){
			local_map[i] = local_map[i+k*rows_to_receive];
			local_map[i+k*(rows_to_receive+1)] = local_map[i+k];
		}

		
		unsigned char left_col[rows_to_receive+2];
		unsigned char right_col[rows_to_receive+2];

		/* Writing left and right columns */
		for (int i =1; i<rows_to_receive+1; i++){
			left_col[i] = local_map[i*k+k-1];
			right_col[i] = local_map[i*k];
		}
		/* Writing corners */
		left_col[0] = local_map[k*rows_to_receive+k-1]; 
		left_col[rows_to_receive+1] = local_map[2*k -1]; 
		right_col[0] = local_map[rows_to_receive*k]; 
		right_col[rows_to_receive+1] = local_map[k]; 

		printf("Process %d: calling static evolution function \n", process_rank);
		/*Perform evolution in parallel*/
		static_evolution(local_map, local_map2, left_col, right_col, rows_to_receive, k);
		printf("Process %d: Returned from static evolution function \n", process_rank);
		/*NOTE: gathering has to be properly implemented*/
		if (process_rank == 0) {

			for (int i = 1; i < size_of_cluster; i++) {
			    	int gather_start_row = i * n_lines_per_process;
				int rows_to_receive = nrows_given_process(i, n_lines_per_process, k);
			    	MPI_Recv(map1_char + gather_start_row * k, rows_to_receive * k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else {
			int gather_start_row = (process_rank == 0) ? 0 : 1;
			int gather_rows_to_send = rows_to_receive;
			printf("Process %d, about to send %d chars to process 0\n", process_rank, gather_rows_to_send*k);
			MPI_Send(local_map + gather_start_row * k, gather_rows_to_send * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD); 
		}
	if (process_rank == 0){
		sprintf(snapshot_name, "images/snapshots/snapshot%d.pgm", i);
        	write_pgm_image(map1_char, maxval, k, k, snapshot_name);
        }
	//}
	// Printing map to debug - we need to find uninitialized cells! 
	MPI_Barrier(MPI_COMM_WORLD);
	if(process_rank ==0)
		print_map(process_rank, rows_to_receive, k, map1_char); 
	MPI_Barrier(MPI_COMM_WORLD);
	printf("Process %d: finished step %d\n", process_rank, i);
    	}

    #ifdef PROFILING
        double tend = CPU_TIME;
        double ex_time = tend-tstart;
        printf("\n\n Execution time is %f\n\n", ex_time);
    #endif
    
/*	printf("Process %d is freeing sub_map at %p\n", process_rank, sub_map); */
	free(sub_map);
	free(local_map);
	free(local_map2);
    	free(map1);
    	if (map2 != NULL)
        	free(map2);

    	if (fname != NULL)
    		free(fname);
	}
    	MPI_Finalize();    

    	return 0;
}
