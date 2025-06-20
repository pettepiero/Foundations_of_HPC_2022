/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */

#include <stdlib.h>
#include <stdio.h> 
#include <stddef.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include <omp.h>
#include <mpi.h>
#include "dynamics.h"

double  tend;
double  tstart;
double  ex_time;
#define NROWS 3000
#define NCOLS 3000
#define NUM_STEPS 500
#define PROB 0.5

void smaller_static_evolution(int num_steps, unsigned char *restrict current, int ncols, int nrows){
    /*Performs a single step of the update of the map*/
        /* Create copy of MPI local map (current) but initialize it with OpenMP to place it in the
        * correct threads */

        unsigned char *sub_map = NULL;
        sub_map = (unsigned char*)malloc(ncols*nrows*sizeof(unsigned char));
        if (sub_map == NULL){
                printf("Could not allocate memory for local maps\n");
                exit(1);
        }

        #pragma omp parallel 
        {
                /* Initialize local sub map with OpenMP, to warm up the data properly*/
                #pragma omp for schedule(static, 4)
                for (int i=0; i<nrows*ncols; i++){
                        sub_map[i] = current[i];
                }

                for(int iter=0; iter < num_steps; iter++){
                        #pragma omp for schedule(static, 4) 
                        for(int row=1; row<nrows-1;row++){ /* Split work with OpenMP over rows */
                                for(int col=1; col < ncols-1; col++){
                                        int i = row*ncols+col;
                                        int alive_counter = count_alive_neighbours_multi(sub_map, ncols, i);
                                        sub_map[i] += UPDATE_CELL(alive_counter);
                                }
                        }
                }

                /* Copy sub_map into current and free sub_map */
                #pragma omp for schedule(static, 4) 
                for (int i=0; i<nrows*ncols; i++){
                        current[i] = sub_map[i];
                }
        }
        free(sub_map);
}


int main(int argc, char** argv)
{
	int nrows = NROWS;
	int ncols = NCOLS + calculate_cache_padding(NCOLS);
	unsigned char *current = (unsigned char*)malloc(nrows*ncols*sizeof(unsigned char));
	/* Randomly initialize matrix */
	printf("Initializing random matrix...\n");
	for(int i=0; i<nrows*ncols; i++){
		float random = (float)rand()/RAND_MAX;	
			if (random<PROB)
				current[i] = 1;
			else
				current[i] = 0;
	}		
	printf("... done\n");

	tstart = CPU_TIME;
	printf("Starting static evolution\n");
	smaller_static_evolution(NUM_STEPS, current, ncols, nrows);
	free(current);
        tend = CPU_TIME;
        ex_time = tend-tstart;
        printf("\n\n%f\n\n", ex_time);

	return 0;
}
