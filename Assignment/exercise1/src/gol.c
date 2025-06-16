/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */

#include <stdlib.h>
#include <stdio.h> 
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include "dynamics.h"
#include <omp.h>
#include <mpi.h>

double 	tend;
double 	tstart;
double  ex_time;

//DEBUG
double 	tend1;
double 	tstart1;
double  ex_time1;

int main(int argc, char** argv)
{

	/**************************************************************************
	 * MPI Initialization
	 **************************************************************************/
	int mpi_provided_thread_level;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
	if ( mpi_provided_thread_level < MPI_THREAD_FUNNELED ) {
		printf("a problem arised when asking for MPI_THREAD_FUNNELED level\n");
		MPI_Finalize();
		exit( 1 );
	}

	Env env;
	int process_rank; 
	void *map1 = NULL;
	unsigned char *map1_char = NULL;
	unsigned char *sub_map = NULL;
	char snapshot_name[100];
	const char *snapshot_folder_path = "images/snapshots";
	char *fname  = NULL;
    	
	MPI_Comm_size(MPI_COMM_WORLD, &env.size_of_cluster);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

	//DEBUG timing set up time
	if (process_rank == 0){
		tstart1 = CPU_TIME;
	}
	
	int next_processor = calculate_next_processor(process_rank, env.size_of_cluster);	
	int prev_processor = calculate_prev_processor(process_rank, env.size_of_cluster);	
	int rows_per_processor[env.size_of_cluster];
	int start_indices[env.size_of_cluster];
	env.my_process_rows = 0;
	env.my_process_start_idx = 0;

		/* Creating an MPI type to send Env variable */
	const int 		nitems = 10;
	int 			blocklengths[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	MPI_Datatype 	types[10] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT}; 
	MPI_Datatype 	mpi_env_type;
	MPI_Aint 		offsets[10];

	offsets[0] = offsetof(Env, action);
	offsets[1] = offsetof(Env, k);
	offsets[2] = offsetof(Env, cache_padding);
	offsets[3] = offsetof(Env, e);
	offsets[4] = offsetof(Env, n);
	offsets[5] = offsetof(Env, s);
	offsets[6] = offsetof(Env, size_of_cluster);
	offsets[7] = offsetof(Env, nrows);
	offsets[8] = offsetof(Env, my_process_rows);
	offsets[9] = offsetof(Env, my_process_start_idx);	
	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_env_type);
	MPI_Type_commit(&mpi_env_type);

	/**************************************************************************
	 * Global setup
	 **************************************************************************/

	/* Things that only process 0 has to do:
	    1. Parse command line arguments
	    2. Read or generate matrix
	    3. Calculate the lines per process*/
	if(process_rank == 0){	
		#ifndef _OPENMP
			printf("\nExecuting without OPENMP in serial mode.\n");
		#else
			#pragma omp parallel
			{
				int id = omp_get_thread_num();
				int num_threads = omp_get_num_threads();
				#pragma omp single
				printf("There are %d threads and I'm thread %d\n", num_threads, id);
			}
		#endif
		printf("Size of MPI cluster: %d\n", env.size_of_cluster);
		initialize_env_variable(&env);
		/* Parse command line arguments */
		command_line_parser(&env, &fname, argc, argv);
		printf("- Matrix size: %d x %d \n- Number of steps: %d\n- e: %d(0=ORDERED, 1=STATIC)\n- Action: %d (1=INIT, 2=RUN)\n"
			"- s: %d\n", env.k, env.k, env.n, env.e, env.action, env.s);
		calculate_rows_per_processor(env, rows_per_processor, start_indices);
		env.my_process_rows = rows_per_processor[process_rank];
		set_up_map_variable(env.action, env.e, env.k, &map1, MAXVAL, fname);
		map1_char = (unsigned char*)map1;

		printf("\n");
		/* Communicate user's choices and other info to other MPI processes */
		for (int i=1; i<env.size_of_cluster; i++){
			MPI_Send(&env, 1, mpi_env_type, i, 0, MPI_COMM_WORLD);
			MPI_Send(rows_per_processor, env.size_of_cluster, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(start_indices, env.size_of_cluster, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
		//deleting old snapshots
		delete_pgm_files(snapshot_folder_path); 
	/* Receive user's choices and other info from process 0*/
	} else if (process_rank != 0){
		MPI_Recv(&env, 1, mpi_env_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&rows_per_processor, env.size_of_cluster, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&start_indices, env.size_of_cluster, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		env.my_process_rows = rows_per_processor[process_rank];
	}

	//DEBUG printing how long set up took
	if(process_rank==0){
		tend1 = CPU_TIME;
		ex_time1 = tend1 - tstart1;
		printf("This setup took approximately %f seconds\n\n", ex_time1);
	}
    

	/* Shift old values of map to MSB 
 	*  this means that 0 becomes 0 and 1 becomes 2^(shift)128 */
	char shift = sizeof(unsigned char)*8 -1;

	/**********************************************************
 	* ORDERED EVOLUTION 
 	* ********************************************************/
	if ((env.e == ORDERED) && (process_rank ==0)){
		tstart= CPU_TIME;
		if (env.s != 0){
			for (int i = 0; i < env.n; i += env.s) {
			    	for (int j = 0; j < env.s && i + j < env.n; j++) {
			    	    ordered_evolution(map1_char, env.k, env.nrows);
			    	}
			    	sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", i + env.s - 1);
			    	write_pgm_image(map1_char, MAXVAL, env.k, env.nrows, snapshot_name);
			}

		} else {
			for(int i = 0; i < env.n; i++){
				ordered_evolution(map1_char, env.k, env.nrows); 		
			}
			sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", env.n);
	  		write_pgm_image(map1_char, MAXVAL, env.k, env.nrows, snapshot_name);
		}
		tend = CPU_TIME;
		ex_time = tend-tstart;
		printf("\n\n%f\n\n", ex_time);

	/**********************************************************
 	* STATIC EVOLUTION 
 	* ********************************************************/
	} else if (env.e == STATIC){
		/*Allocate space for local maps*/
		sub_map = (unsigned char*)malloc(env.my_process_rows*env.k*sizeof(unsigned char));
		if (sub_map == NULL){
			printf("Process %d error: Could not allocate memory for local maps\n", process_rank);
			exit(1);
		}

		/* Initialize local sub matrices with OpenMP, to warm up the data properly*/
		#ifdef _OPENMP
			#pragma omp parallel for schedule(static, 4)
		#endif
		for (int i=0; i<env.my_process_rows*env.k; i++){
			sub_map[i] = 0;	
		}

		/* Process 0 sends initial submap to every MPI process */	
		if (process_rank == 0){
			tstart = CPU_TIME;
			for (int rank = 1; rank < env.size_of_cluster; rank++) {
				MPI_Send(map1_char + start_indices[rank]*env.k, rows_per_processor[rank]*env.k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD);
			}
		//	memcpy(sub_map + start_indices[process_rank]*env.k, map1_char, env.my_process_rows*env.k);
			memcpy(sub_map, map1_char + start_indices[process_rank]*env.k, env.my_process_rows * env.k);

		} else {
			MPI_Recv(sub_map, env.my_process_rows * env.k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		/* Each process now has its submap in variable sub_map*/

		/*DEBUG: printing when everybody is ready for loop */
		MPI_Barrier(MPI_COMM_WORLD);
		if (process_rank == 0){
			tend1 = CPU_TIME;
			ex_time1 = tend1 - tstart1;
			printf("After %f seconds everybody is ready for the loop\n\n", ex_time1);
		}

		/* Loop scenario in which a screenshot has to be taken every s steps of the evolution*/
		if (env.s != 0){
			/* Outer loop, takes screenshot at each iteration */
			for (int i = 0; i < env.n; i += env.s) {
				/* Inner loop, iterations that do not need screenshot */
				for (int j = 0; j < env.s && i + j < env.n; j++) {
					MPI_Request requests[4];

					MPI_Irecv(&sub_map[0], env.k, MPI_UNSIGNED_CHAR, prev_processor, 0, MPI_COMM_WORLD, &requests[0]); // top ghost
					MPI_Irecv(&sub_map[(env.my_process_rows - 1) * env.k], env.k, MPI_UNSIGNED_CHAR, next_processor, 1, MPI_COMM_WORLD, &requests[1]); // bottom ghost
					MPI_Isend(&sub_map[1 * env.k], env.k, MPI_UNSIGNED_CHAR, prev_processor, 1, MPI_COMM_WORLD, &requests[2]); // my first real row
					MPI_Isend(&sub_map[(env.my_process_rows - 2) * env.k], env.k, MPI_UNSIGNED_CHAR, next_processor, 0, MPI_COMM_WORLD, &requests[3]); // my last real row

					//shift_old_map(sub_map, env.k, env.my_process_rows, shift);
					static_evolution_inner(sub_map, env.k, env.my_process_rows, shift);
					MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);

					static_evolution_border(sub_map, env.k, env.my_process_rows, shift);
		        	}

				/* Process 0 takes screenshot */
				if (process_rank == 0){
					gather_submaps(map1_char, env, start_indices, rows_per_processor);
					//memcpy(map1_char + start_indices[process_rank] * env.k, sub_map, env.my_process_rows * env.k);
					//memcpy(map1_char, sub_map, env.my_process_rows);
					memcpy(map1_char + (start_indices[process_rank] + 1) * env.k, sub_map + env.k, (env.my_process_rows - 2) * env.k);

					update_horizontal_edges(map1_char, env.k, env.nrows);
					sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", i + env.s - 1);
					write_pgm_image(map1_char, MAXVAL, env.k, env.nrows, snapshot_name);
   				} else {
					send_submaps(sub_map, env, rows_per_processor, process_rank);
				}
			}

		} else { /* In this case the whole evolution is performed without snapshots until the very end */ 
			if (process_rank == 0){
				printf("DEBUG: Starting evolution with env.k = %d\n", env.k);
			}
	    		for (int i = 0; i < env.n; i++) {
			//	// Perform evolution in parallel
			//	MPI_Request requests[4];
    			//	// Exchange ghost rows with neighbors
    			//	// Receive top ghost row (row 0 of sub_map) from above
    			//	MPI_Irecv(&sub_map[0], env.k, MPI_UNSIGNED_CHAR, prev_processor, 0, MPI_COMM_WORLD, &requests[0]);
    			//	// Receive bottom ghost row (last row of sub_map) from below
    			//	MPI_Irecv(&sub_map[(env.my_process_rows - 1) * env.k], env.k, MPI_UNSIGNED_CHAR, next_processor, 1, MPI_COMM_WORLD, &requests[1]);
    			//	// Send my first real row (row 1) to above (they'll store it as their bottom ghost)
    			//	MPI_Isend(&sub_map[1 * env.k], env.k, MPI_UNSIGNED_CHAR, prev_processor, 1, MPI_COMM_WORLD, &requests[2]);
    			//	// Send my last real row (row my_process_rows - 2) to below (they'll store it as top ghost)
    			//	MPI_Isend(&sub_map[(env.my_process_rows - 2) * env.k], env.k, MPI_UNSIGNED_CHAR, next_processor, 0, MPI_COMM_WORLD, &requests[3]);
    			//	// Compute inner rows (that don't depend on ghost rows)
    			//	// Rows: 2 to (my_process_rows - 3), inclusive
			//	double t0 = omp_get_wtime();

			//	shift_old_map(sub_map, env.k, env.my_process_rows, shift);
    			//	static_evolution_inner(sub_map, env.k, env.my_process_rows, shift);
			//	double t1 = omp_get_wtime();
			//	if (process_rank == 0) printf("Time: %f seconds\n", t1 - t0);
    			//	// Wait for halo exchanges to complete before border updates

    			//	MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
    			//	// Compute top and bottom border rows (row 1 and row my_process_rows - 2)
    			//	static_evolution_border(sub_map, env.k, env.my_process_rows, shift);
    			//	// Copy new map into current
	
			}
			if (process_rank == 0){
				gather_submaps(map1_char, env, start_indices, rows_per_processor);
				memcpy(map1_char + start_indices[process_rank] * env.k, sub_map, env.my_process_rows * env.k);
			    	tend = CPU_TIME;
			    	ex_time = tend-tstart;
				printf("%d steps of the evolution took %f seconds\n", env.n, ex_time);
				printf("\n\n%f\n\n", ex_time);
				sprintf(snapshot_name, "images/snapshots/snapshot_%05d.pgm", env.n - 1);
				convert_map_to_char(map1_char, env.k, env.nrows);
				write_pgm_image(map1_char, MAXVAL, env.k, env.nrows, snapshot_name);
	   		} else {
				send_submaps(sub_map, env, rows_per_processor, process_rank);
			}
		}

		/* Free sub_map in each process */
		free(sub_map);
		sub_map = NULL;
	}

	/* Free map1 only in process 0 */
	if (process_rank == 0) {
		free(map1);
		map1 = NULL;
		tend = CPU_TIME;
		ex_time = tend-tstart;
		printf("\n\n%f\n\n", ex_time);
	}
	MPI_Type_free(&mpi_env_type);
	MPI_Finalize();

	return 0;
}
