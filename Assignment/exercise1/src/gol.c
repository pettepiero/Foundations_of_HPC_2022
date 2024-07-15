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

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    int size_of_cluster;
    int process_rank;
    int n_lines_per_process;
    int rows_to_send = 0;
    void *map1 = NULL;
    void *map2 = NULL;
    unsigned char *map1_char = NULL;
    unsigned char *sub_map = NULL;
    char file[] = "images/blinker.pgm";

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
        if (e == STATIC){
            static_set_up_other_map(map1, map2, k);
        } else if (e == ORDERED)
            init_to_zero(map1_char, k);
    }
    /*Make every process wait for process 0 to perform the task above*/
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(&n_lines_per_process, 1, MPI_INT, 0, MPI_COMM_WORLD);  

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
        // for(int i = 0; i < N_STEPS; i++){
        //     ordered_evolution(map1, k, rows_to_send);
        //     #ifndef PROFILING
        //         sprintf(fname, "images/snapshots/snapshot%d.pgm", i);
        //         write_pgm_image(map1, maxval, k, k, fname);
        //     #endif
        // }
        exit(1);

    } else if (e == STATIC){
        for(int i = 0; i < N_STEPS; i++){

			/* Not using MPI_Scatter because I want to specify the rows to send exactly. */

		if (process_rank ==0){
			for (int i = 0; i < size_of_cluster; i++){
				int start_row = i * (n_lines_per_process + 1);
				int end_row = start_row + n_lines_per_process - 1;
				if (end_row >= k)
					end_row = k- 1; // Limit end_row to the matrix size
				int rows_to_copy = end_row - start_row + 1;
				MPI_Send(map1_char + start_row * k, rows_to_copy * k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
			}
		}

		int start_row = process_rank * (n_lines_per_process + 1);
		int end_row = start_row + rows_to_send - 1;

		if (end_row >= k)
			end_row = k- 1; // Limit end_row to the matrix size

		int rows_to_receive = end_row - start_row + 1;

		//Allocate space for local maps
		sub_map = (unsigned char*)malloc(rows_to_receive * k * sizeof(unsigned char));
		if (sub_map == NULL){
			printf("Error: Could not allocate memory for local maps\n");
			exit(1);
		}

		MPI_Recv(sub_map, rows_to_receive * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		printf("Process %d received the submatrix\n", process_rank);
		
		printf("Rank %d - Saving received matrix to local variables\n", process_rank);

		unsigned char* local_map = malloc((rows_to_receive+2)*k*sizeof(unsigned char));
		unsigned char* local_map2 = malloc((rows_to_receive+2)*k*sizeof(unsigned char));

		if(local_map == NULL || local_map2 == NULL){
			printf("Rank %d - Error in allocating local maps\n\n", process_rank);
			exit(1);
		}

		int index;
		for (int i=0; i<rows_to_receive; i++){
			for(int j=0; j<k; j++){
				index = i*k+j;
				local_map[index+k] = sub_map[index];	
			}
		}

		printf("Rank %d - Setting up borders of local copy\n", process_rank);	

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

		/*Perform evolution in parallel*/
		static_evolution(local_map, local_map2, left_col, right_col, rows_to_receive, k);

		printf("Process %d finished the step\n", process_rank);

		/*NOTE: gathering has to be properly implemented*/
		if (process_rank == 0) {
			printf("Process %d: gathering the results\n", process_rank);     

			for (int i = 0; i < size_of_cluster; i++) {
			    int gather_start_row = i * n_lines_per_process;
			    int gather_rows_to_receive = n_lines_per_process;
			    MPI_Recv(map1_char + gather_start_row * k, gather_rows_to_receive * k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else {
			int gather_start_row = (process_rank == 0) ? 0 : 1;
			int gather_rows_to_send = n_lines_per_process;
			MPI_Send(local_map + gather_start_row * k, gather_rows_to_send * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
		}

		if (process_rank == 0) {
			printf("Received full matrix\n");
		}
	
		free(sub_map);
		free(local_map);
        }
    }

    #ifdef PROFILING
        double tend = CPU_TIME;
        double ex_time = tend-tstart;
        printf("\n\n Execution time is %f\n\n", ex_time);
    #endif
    
    free(map1);
    if (e == STATIC)
        free(map2);

    free(fname);
        
    MPI_Finalize();    
    return 0;
}
