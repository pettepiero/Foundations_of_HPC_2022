#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include <omp.h>
#include <mpi.h>


#define IS_ALIVE(value) (((value) & 0x80) ? 1 : 0)


void print_map_2(int process_rank, int ncols, int rows_to_receive, unsigned char *map)
{

	printf("Process %d:\n", process_rank);
	for (int i = 0; i < rows_to_receive; i++)
	{
		for (int j = 0; j < ncols; j++)
			printf("%4d", map[i * ncols + j]);
		printf("\n");
	}
}
void init_to_zero(unsigned char *restrict map1, const int k)
{
    #ifdef _OPENMP
        // Touching the maps in the different threads,
        // so that each cache will be warmed-up appropriately
        #pragma omp parallel
    #endif
        {
            for (int i=0; i<k; i++){
                for(int j = 0; j <k; j++){
                    map1[i*k +j] = 0;
                }
            }
        }
}

void update_horizontal_edges(unsigned char *restrict map, const int ncols, const int nrows)
{
    if (nrows < 3) return; // Sanity check for small matrices
    // update top and bottom edges
    for (int i=0; i<ncols; i++)
    {
        map[i] = map[i+(nrows-2)*ncols];
        map[i+(nrows-1)*ncols] = map[i+ncols];
    }
}


void print_map_to_file(unsigned char *restrict map, const int ncols, const int nrows, const char fileName[])
{
    FILE *f = fopen(fileName, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fclose(f);

    f = fopen(fileName, "a");
    for (int i=0; i<nrows; i++){
        for(int j = 0; j <ncols; j++){
            fprintf(f, "%4d", map[i*ncols+j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);

}

int is_alive(const unsigned char *value){
	//return (*value & 0x80) >> 7; 
	return (*value >> 7);
}

// Defenitely the wrong way to do it but it's a start
//IDEA: Use a lookup table for the neighbours ? 
//IDEA: would it be faster to calculate neighbours for more than one position, therefore reducing #function calls?
//NOTE: receives shifted map!! alive neighbours if its MSB is 1, dead if 0
int count_alive_neighbours_multi(const unsigned char *restrict map, const int ncols, const int index)
{
	// NOTE: indexes that are passed should not be on the edges!
	// Precompute the row offsets
	int prev_row = index - ncols;
	int next_row = index + ncols;
	int count0 = is_alive(&map[index - 1]) + is_alive(&map[index + 1]);
	int count1 = is_alive(&map[prev_row]) + is_alive(&map[prev_row - 1]) + is_alive(&map[prev_row + 1]);
	int count2 = is_alive(&map[next_row]) + is_alive(&map[next_row - 1]) + is_alive(&map[next_row + 1]);
	return count0+count1+count2;  
}

int count_alive_neighbours_ordered(const unsigned char *restrict map, const int ncols, const int index)
{
	// NOTE: indexes that are passed should not be on the edges!
	// Precompute the row offsets
	int prev_row = index - ncols;
	int next_row = index + ncols;
	int count0 = map[index - 1] + map[index + 1];
	int count1 = map[prev_row] + map[prev_row - 1] + map[prev_row + 1];
	int count2 = map[next_row] + map[next_row - 1] + map[next_row + 1];
	return count0+count1+count2;  
}
int count_alive_neighbours_single(const unsigned char *restrict map, const int ncols, const int index)
{
	int count = 0;
    	int neighbours[] = {index-1, index+1, index-ncols, index+ncols, 
    	                    index-ncols-1, index-ncols+1, index+ncols-1, index+ncols+1};

    	for (int i = 0; i < 8; i++) {
    	    count += is_alive(&map[neighbours[i]]);
    	}
    	return count;
}

char update_cell(const int alive_neighbours) {
/* Returns MAXVAL if #alive neighbours is 2 or 3, 0 otherwise */ 
    return (alive_neighbours == 2 || alive_neighbours == 3) ? MAXVAL : 0;
}

void ordered_evolution(int num_steps, unsigned char *restrict map, int ncols, int nrows)
{   
	unsigned char *copy_of_map = NULL;
	copy_of_map = (unsigned char*)malloc(ncols*nrows*sizeof(unsigned char));
	if (copy_of_map == NULL){
		printf("Could not allocate memory for local maps\n");
		exit(1);
	}
	#pragma omp parallel
	{
		int i = 0;
		int alive_counter = 0;	

		/* Initialize local copy of map with OpenMP, to warmp up the data properly */
		# pragma omp for schedule(static)
		for (int i=0; i<nrows*ncols; i++){
			copy_of_map[i] = map[i];
		}

		for (int iter = 0; iter < num_steps; iter++) {
			#pragma omp for schedule(static) 
	    		for (int row = 1; row < nrows-1; row++){
				for (int col = 0; col < ncols; col++){
					 i = row*ncols+ col;
				   	 alive_counter = count_alive_neighbours_ordered(copy_of_map, ncols, i);
				   	 copy_of_map[i] = UPDATE_CELL(alive_counter);
				}
				int left_border_counter = 0;
				int right_border_counter = 0;

				/*Count alive neighbours for left and right
  				 border elements */

				int i = row*ncols;
				left_border_counter = 0;
				right_border_counter = 0;

				left_border_counter += copy_of_map[i-1];
				left_border_counter += copy_of_map[i+1];
				left_border_counter += copy_of_map[i+ncols-1];
				left_border_counter += copy_of_map[i+ncols+1];
				left_border_counter += copy_of_map[i+ncols];
				left_border_counter += copy_of_map[i+2*ncols-1]	;
				left_border_counter += copy_of_map[i-ncols];
				left_border_counter += copy_of_map[i-ncols+1];

				copy_of_map[i] += UPDATE_CELL(left_border_counter);

				i += ncols -1;
				right_border_counter += copy_of_map[i+1];
				right_border_counter += copy_of_map[i-1];
				right_border_counter += copy_of_map[i+ncols-1];
				right_border_counter += copy_of_map[i+ncols];
				right_border_counter += copy_of_map[i-2*ncols+1];
				right_border_counter += copy_of_map[i-ncols+1];
				right_border_counter += copy_of_map[i-ncols-1];
				right_border_counter += copy_of_map[i-ncols];
			
				copy_of_map[i] += UPDATE_CELL(right_border_counter);
	    		}
		}
		/* Copy copy_of_map into map and free copy_of_map */
		#pragma omp for schedule(static)
		for(int i=0; i<nrows*ncols; i++){
			map[i] = copy_of_map[i];
		}
	}
	free(copy_of_map);
}


void shift_old_map(unsigned char *restrict map, const int ncols, const int nrows, const char shift)
/* Shifts the map by the specified number of positions. */
{
	int i = 0;
	#ifdef _OPENMP
		#pragma omp for schedule(static) private(i)
	#endif
	for(int row=0; row < nrows ; row++){
		for(int col=0; col < ncols; col++){
			i = row*ncols + col;			
			map[i] = map[i] << shift;
		}
	}
}

void mask_MSB(unsigned char *restrict map, const int ncols, const int nrows)
{	
	/* Sets MSB of each element of map to 0 */
	int i = 0;
	#ifdef _OPENMP
		#pragma omp for schedule(static) private(i)
	#endif
	for(int row=0; row < nrows ; row++){
		for(int col=0; col < ncols; col++){
			i = row*ncols + col;			
			map[i] &= 0x7F;
		}
	}
}

void static_evolution(int num_steps, unsigned char *restrict current, int ncols, int nrows, char shift, int prev_processor, int next_processor){
	shift_old_map(current, ncols, nrows, shift);	
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
		#pragma omp for schedule(static)
		for (int i=0; i<nrows*ncols; i++){
			sub_map[i] = current[i];	
		}

		for(int iter=0; iter < num_steps; iter++){

			#pragma omp for schedule(static) 
			for(int row=1; row<nrows-1;row++){ /* Split work with OpenMP over rows */
				for(int col=1; col < ncols-1; col++){
					int i = row*ncols+col;
					int alive_counter = count_alive_neighbours_multi(sub_map, ncols, i);
					sub_map[i] += UPDATE_CELL(alive_counter);
				}
				int left_border_counter = 0;
				int right_border_counter = 0;

				/*Count alive neighbours for left and right
  				 border elements */

				int i = row*ncols;
				left_border_counter = 0;
				right_border_counter = 0;

				left_border_counter += is_alive(&sub_map[i-1]);	
				left_border_counter += is_alive(&sub_map[i+1]);
				left_border_counter += is_alive(&sub_map[i+ncols-1]);
				left_border_counter += is_alive(&sub_map[i+ncols+1]);
				left_border_counter += is_alive(&sub_map[i+ncols]);
				left_border_counter += is_alive(&sub_map[i+2*ncols-1]);	
				left_border_counter += is_alive(&sub_map[i-ncols]);
				left_border_counter += is_alive(&sub_map[i-ncols+1]);
				sub_map[i] += UPDATE_CELL(left_border_counter);

				i += ncols -1;
				right_border_counter += is_alive(&sub_map[i+1]);	
				right_border_counter += is_alive(&sub_map[i-1]);
				right_border_counter += is_alive(&sub_map[i+ncols-1]);
				right_border_counter += is_alive(&sub_map[i+ncols]);	
				right_border_counter += is_alive(&sub_map[i-2*ncols+1]);	
				right_border_counter += is_alive(&sub_map[i-ncols+1]);	
				right_border_counter += is_alive(&sub_map[i-ncols-1]);	
				right_border_counter += is_alive(&sub_map[i-ncols]);	
				
				sub_map[i] += UPDATE_CELL(right_border_counter);
				
				/* MPI Communication */
			}

			#pragma omp master
			{
                		MPI_Sendrecv(sub_map + ncols, ncols, MPI_UNSIGNED_CHAR, prev_processor, 0, sub_map + (nrows - 1) * ncols, ncols, MPI_UNSIGNED_CHAR, next_processor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                		MPI_Sendrecv(sub_map + (nrows - 2) * ncols, ncols, MPI_UNSIGNED_CHAR, next_processor, 0, sub_map, ncols, MPI_UNSIGNED_CHAR, prev_processor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			} 
			#pragma omp barrier
		}

		/* Copy sub_map into current and free sub_map */	
		#pragma omp for schedule(static) 
		for (int i=0; i<nrows*ncols; i++){
			current[i] = sub_map[i];	
		}
	}

	free(sub_map);
	/* Keep only LSB */
	mask_MSB(current, ncols, nrows);
}

void gather_submaps(unsigned char* restrict map, const Env env, const int *start_indices, const int *rows_per_processor){
	for(int rank=1; rank < env.size_of_cluster; rank++){
		MPI_Recv(map + (start_indices[rank]+1)*env.k, (rows_per_processor[rank]-2)*env.k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
}


void send_submaps(unsigned char* restrict map, const Env env, const int *rows_per_processor, const int process_rank){
	MPI_Send(map +env.k, (env.my_process_rows-2)*env.k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
}

