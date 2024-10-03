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

int   	action = 0;
int   	k      = K_DFLT;
int   	e      = STATIC;
int   	n      = N_STEPS;
int   	s      = 1;
double 	tend;
double 	tstart;
double  ex_time; 
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
	int nrows = 0;
    	void *map1 = NULL;
    	unsigned char *map1_char = NULL;
    	unsigned char *sub_map = NULL;
    	unsigned char *sub_map_copy = NULL;
    	char snapshot_name[100];
	const char *snapshot_folder_path = "images/snapshots";
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
        	#endif
		printf("Size of MPI cluster: %d\n", size_of_cluster);
        	command_line_parser(&action, &k, &e, &fname, &n, &s, argc, argv);
		printf("- Matrix size: %d\n- Number of steps: %d\n- e: %d(0=ORDERED, 1=STATIC)\n- Action: %d (1=INIT, 2=RUN)\n"
			"- s: %d\n", k, n, e, action, s);
		nrows = k+2;
		calculate_rows_per_processor(nrows, size_of_cluster, rows_per_processor, start_indices);
			
		set_up_map_variable(action, e, k, &map1, MAXVAL, fname);
        	map1_char = (unsigned char*)map1;
		
        	/*if (e == STATIC){
        	    static_set_up_other_map(map1, &map2, k);
        	} */
		my_process_rows = rows_per_processor[0];
		my_process_start_idx = start_indices[0];
		for (int i=1; i<size_of_cluster; i++){
			MPI_Send(&e, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&rows_per_processor[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&start_indices[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			MPI_Send(&k, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
			MPI_Send(&n, 1, MPI_INT, i, 0, MPI_COMM_WORLD); 
		}
		//deleting old snapshots
		delete_pgm_files(snapshot_folder_path); 
		
    	} else if (process_rank != 0){
		MPI_Recv(&e, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&my_process_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		MPI_Recv(&my_process_start_idx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		MPI_Recv(&k, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
	}

    	if ((e == ORDERED) && (process_rank ==0)){
		tstart= CPU_TIME;
		if (s != 0){
			for (int i = 0; i < n; i += s) {
			    for (int j = 0; j < s && i + j < n; j++) {
			        ordered_evolution(map1_char, k, nrows);
			    }
			    sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", i + s - 1);
			    write_pgm_image(map1_char, MAXVAL, k, nrows, snapshot_name);
			}


/*			for(int i = 0; i < n; i++){
				ordered_evolution(map1_char, k, nrows); 		
				step_counter++;
				if (step_counter == s){
					sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", i);
	  				write_pgm_image(map1_char, MAXVAL, k, nrows, snapshot_name);
					step_counter = 0;
				}
			}
*/		} else {
			for(int i = 0; i < n; i++){
				ordered_evolution(map1_char, k, nrows); 		
			}
			sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", n);
	  		write_pgm_image(map1_char, MAXVAL, k, nrows, snapshot_name);
		}
		tend = CPU_TIME;
		ex_time = tend-tstart;
		printf("\n\n%f\n\n", ex_time);

    } else if (e == STATIC){
		//Allocate space for local maps
		sub_map = (unsigned char*)malloc(my_process_rows*k*sizeof(unsigned char));
		sub_map_copy = (unsigned char*)malloc(my_process_rows*k*sizeof(unsigned char));
		
		if (sub_map == NULL || sub_map_copy == NULL){
			printf("Process %d error: Could not allocate memory for local maps\n", process_rank);
			exit(1);
		}

		/* Initialize local sub matrices with OpenMP, to warm up the data properly*/
		#pragma omp parallel for
		for (int i=0; i<my_process_rows*k; i++){
			sub_map[i] = 0;	
			sub_map_copy[i]=0;
		}	
	
		if (process_rank == 0){
			tstart= CPU_TIME;
			if (s != 0){
				/* Loop every s steps for snapshot */
				for (int i = 0; i < n; i += s) {
					/* Loop over number of steps without snapshot */
					for (int j = 0; j < s && i + j < n; j++) {
						/* Loop over MPI cluster */
			        		for (int rank = 1; rank < size_of_cluster; rank++) {
			        			MPI_Send(map1_char + start_indices[rank]*k, rows_per_processor[rank]*k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD);
			        		}
			        		memcpy(sub_map, map1_char, k*my_process_rows);

			        		// Perform evolution in parallel
			        		static_evolution(sub_map, k, my_process_rows);
    
			        		// Gathering the results from other processes.
			        		for (int rank = 1; rank < size_of_cluster; rank++) {
			        			MPI_Recv(map1_char + (start_indices[rank]+1)*k, (rows_per_processor[rank]-2)*k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			        		}
			        		memcpy(map1_char, sub_map, k*my_process_rows);
			        		update_horizontal_edges(map1_char, k, nrows);
   					}
					/* Steps without snapshots are over */
			    		sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", i + s - 1);
			    		write_pgm_image(map1_char, MAXVAL, k, nrows, snapshot_name);
				}
			} else { /* In this case the whole evolution is performed without snapshots until the very end */ 
    		    		for (int i = 0; i < n; i++) {
		        		// Scattering the map
		        		for (int rank = 1; rank < size_of_cluster; rank++) {
		        			MPI_Send(map1_char + start_indices[rank]*k, rows_per_processor[rank]*k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD);
		        		}
		        		memcpy(sub_map, map1_char, k*my_process_rows);
		        		
		        		// Perform evolution in parallel
		        		static_evolution(sub_map, k, my_process_rows);
		        		
		        		// Gathering the results from other processes
		        		for (int rank = 1; rank < size_of_cluster; rank++) {
		        			MPI_Recv(map1_char + (start_indices[rank]+1)*k, (rows_per_processor[rank]-2)*k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		        		}
		        		memcpy(map1_char, sub_map, k*my_process_rows);
		       		} 
			    	sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", n - 1);
				convert_map_to_char(map1_char, k, nrows);
			    	write_pgm_image(map1_char, MAXVAL, k, nrows, snapshot_name);
   			}
		} else {
			// Non-zero ranks handle receiving, evolving, and sending data.
		    	for (int i = 0; i < n; i++) {
		    		// Receive the map segment from rank 0
		    	    	MPI_Recv(sub_map, my_process_rows * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		    	    	// Perform evolution in parallel
		    	    	static_evolution(sub_map, k, my_process_rows);
		
		    	    	// Send only the inner rows back to rank 0
		    	    	MPI_Send(sub_map + k, (my_process_rows - 2) * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
		    	}
		}

		if (process_rank == 0){
		        tend = CPU_TIME;
		        ex_time = tend-tstart;
		        printf("\n\n%f\n\n", ex_time);
		}
		    
		/* Free sub_map and sub_map_copy in each process */
		free(sub_map);
		free(sub_map_copy);
		sub_map = NULL;
		sub_map_copy = NULL;
	}

	/* Free map1 and map2 only in process 0 */
	if (process_rank == 0) {
		free(map1);
		map1 = NULL;
	}
	MPI_Finalize();

    return 0;
}
