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

int   	action = 0;
int   	k      = K_DFLT;
int   	e      = STATIC;
int   	n      = 10000;
int   	s      = 1;
int 	maxval = 255; //255 -> white, 0 -> black

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
	int nrows;
    	void *map1 = NULL;
    	void *map2 = NULL;
    	unsigned char *map1_char = NULL;
    	unsigned char *sub_map = NULL;
    	unsigned char *sub_map_copy = NULL;
    	char file[] = "images/blinker.pgm";
    	char snapshot_name[100];

	char *fname  = NULL;
    	
	MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
    	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	
	int rows_per_processor[size_of_cluster];
	int start_indices[size_of_cluster];
	int my_process_rows = 0;
	int my_process_start_idx = 0;
    
    /* Things that only process 0 has to do:
        1. Parse command line arguments
        2. Read or generate matrix
        3. Calculate the lines per process*/
    	if(process_rank == 0){
	        #ifndef _OPENMP
	        	printf("\nExecuting without OPENMP in serial mode.\n");
			printf("Size of cluster: %d\n", size_of_cluster);
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
		nrows = k+2;
		calculate_rows_per_processor(nrows, size_of_cluster, rows_per_processor, start_indices);
	
		printf("rows_per_processor = \n");
		for (int j =0; j<size_of_cluster; j++){
			printf("%d, ", rows_per_processor[j]);
		}
		printf("\n");

		printf("start_indices = \n");
		for (int j =0; j<size_of_cluster; j++){
			printf("%d, ", start_indices[j]);
		}
		printf("\n");
	
		set_up_map_variable(action, e, k, &map1, maxval, file);
	        map1_char = (unsigned char*)map1;
		
	        if (e == STATIC){
	            static_set_up_other_map(map1, &map2, k);
	        } else if (e == ORDERED){
	            /*init_to_zero(map1_char, k); need to adjust to new matrix size
	 * 		ncols*nrows */
		}
		my_process_rows = rows_per_processor[0];
		my_process_start_idx = start_indices[0];

		for (int i=1; i<size_of_cluster; i++){
			MPI_Send(&rows_per_processor[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&start_indices[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			MPI_Send(&k, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
		}
		
    	} else {
		MPI_Recv(&my_process_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		MPI_Recv(&my_process_start_idx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		MPI_Recv(&k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		printf("Process %d receives %d rows \n", process_rank, my_process_rows); 
	}
	
    if (e == ORDERED){
        printf("\nOrdered evolution to be implemented with MPI\n");
        exit(1);
    } else if (e == STATIC){
		//Allocate space for local maps
		sub_map = (unsigned char*)calloc(my_process_rows*k, sizeof(unsigned char));
		sub_map_copy = (unsigned char*)calloc(my_process_rows*k, sizeof(unsigned char));
		
		printf("Process %d allocated sub_map at %p\n", process_rank, sub_map);
		printf("Process %d: size of sub_map= %d\n", process_rank, my_process_rows*k);
		printf("Process %d: addresses of sub_map=%p to %p\n", process_rank, sub_map, &sub_map[my_process_rows*k-1]);
		if (sub_map == NULL || sub_map_copy == NULL){
			printf("Process %d error: Could not allocate memory for local maps\n", process_rank);
			exit(1);
		}

		for(int i = 0; i < N_STEPS; i++){
			/* 1) Scattering the map 
			 * Not using MPI_Scatter because I want to specify the rows to send exactly. */
			if (process_rank ==0){
				printf("Starting step %d\n", i);
				for (int i = 1; i < size_of_cluster; i++){
					MPI_Send(map1_char + start_indices[i]*k, rows_per_processor[i]*k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
				}
				memcpy(sub_map, map1_char, k*my_process_rows);
			} else {
				MPI_Recv(sub_map, my_process_rows*k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			} 

			/*Perform evolution in parallel*/
			if (process_rank == 0){
				printf("\n\nAlso process 0 is calling static_evolution\n");
				printf("process 0 will call static_evolution on sub_map=%p and sub_map_copy=%p"
					"\n for %d columns and %d rows\n", sub_map, sub_map_copy, k, my_process_rows);
				printf("i.e. from address %p to address %p\n\n", sub_map, sub_map + k*my_process_rows);
			}

			static_evolution(sub_map, sub_map_copy, k, my_process_rows);

			if (process_rank == 0) {
				for (int j = 1; j < size_of_cluster; j++) {
			/* NOTE: receiving only inner rows, not the out-of-date boundaries used only for computation! */
				    	MPI_Recv(map1_char + (start_indices[j]+1)*k, (rows_per_processor[j]-2)*k, MPI_UNSIGNED_CHAR, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					printf("Process %d, step %d: receiving from process %d and writing from address %p\n"
						"i.e. %d posisions after map1_char = %p (start_indices=%d)\n", process_rank, i, j, map1_char + (start_indices[j]+1)*k, (start_indices[j]+1)*k, map1_char, start_indices[j]);
				}
				printf("Writing process 0 sub_map to map1_char\n"
					"i.e from address %p to address %p\n", map1_char+k, map1_char+k + k*my_process_rows);
				memcpy(map1_char +k, sub_map, k*my_process_rows);
			} else {
			/* NOTE: sending only inner rows, not the out-of-date boundaries used only for computation! */
				MPI_Send(sub_map +k, (my_process_rows-2)*k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD); 
			}

			if (process_rank == 0){
				update_horizontal_edges(map1_char, k, nrows);
				sprintf(snapshot_name, "images/snapshots/snapshot%d.pgm", i);
		        	write_pgm_image(map1_char, maxval, k, nrows, snapshot_name);
		        }
    		}
		if (process_rank == 0){
			#ifdef PROFILING
		        double tend = CPU_TIME;
		        double ex_time = tend-tstart;
		        printf("\n\n Execution time is %f\n\n", ex_time);
			#endif
		}

		    
		MPI_Barrier(MPI_COMM_WORLD);

		// Free sub_map and sub_map_copy in each process
		free(sub_map);
		free(sub_map_copy);
		sub_map = NULL;
		sub_map_copy = NULL;
	}

	// Free map1 and map2 only in process 0
	if (process_rank == 0) {
		printf("Process %d: map1 = %p, map2 = %p\n", process_rank, map1, map2);
		free(map1);
		free(map2);
		map2 = NULL;
		map1 = NULL;
	}

	MPI_Barrier(MPI_COMM_WORLD);

	// Finalize MPI only once for all processes
	MPI_Finalize();
	printf("Process %d finalized\n", process_rank);

    return 0;
}
