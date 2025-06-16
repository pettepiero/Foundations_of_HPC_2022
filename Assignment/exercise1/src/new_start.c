#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

int main(int argc, char** argv){
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
	int process_rank; 
    	int size_of_cluster;	

	MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

	printf("Process rank %d - MPI size of cluster: %d\n", process_rank, size_of_cluster);

	/*************************************************************************
 	* Getting command line arguments
 	* *********************************************************************** */
	int num_elements = 100;
	if (argc > 1) {
		num_elements = atoi(argv[1]);
		if (num_elements <= 0) {
		       	fprintf(stderr, "Invalid number of elements. Must be > 0.\n");
			exit(1);
		}
	}

	/**************************************************************************
	 * OpenMP Initialization 
	 **************************************************************************/
	int num_threads;

	#pragma omp parallel
	{
		#pragma omp master
		{
			num_threads = omp_get_num_threads();
			printf("OpenMP is using %d threads\n", num_threads);
		}	
	}
	/**************************************************************************
	 * Map Initialization and computation
	 **************************************************************************/
	double *map = malloc(num_elements * sizeof(double));
	if (!map) {
	    	fprintf(stderr, "Process %d - malloc failed for %zu bytes\n", process_rank, num_elements * sizeof(double));
		MPI_Finalize();
		exit(1);
	}

	printf("Process rank %d - address of map: %p\n", process_rank, map);
	
	//OpenMP initialization for cache warm up
	#pragma omp parallel for
	for(int i=0; i<num_elements; i++){
		map[i] = (double)i*i;
	}
	double sum = 0.0;
	#pragma omp parallel
	{
		int tid = omp_get_thread_num();
		double local_start = omp_get_wtime();
		
		#pragma omp for reduction(+:sum)
		for (int i = 0; i < num_elements; i++) {
			for (int j = 0; j < 1000; j++) {
    				sum += map[i] * map[i] + map[i] * 2 - map[i] / 3 + map[i] * map[i] * map[i];
			}

		}
		
		double local_end = omp_get_wtime();
		printf("Thread %d ran for %f seconds\n", tid, local_end - local_start);
	}

	free(map);
	MPI_Finalize();
}
