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

#define UPDATE_CELL(count) ((count == 3 || count == 2) ? MAXVAL : 0) // Example logic

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

int is_alive(unsigned char *value){
	return (*value & 0x80) >> (sizeof(unsigned char)*8 -1);
}

// Defenitely the wrong way to do it but it's a start
//IDEA: Use a lookup table for the neighbours ? 
//IDEA: would it be faster to calculate neighbours for more than one position, therefore reducing #function calls?
//NOTE: receives shifted map!! alive neighbours if its MSB is 1, dead if 0
int count_alive_neighbours_multi(const unsigned char *restrict map, const int ncols, const int index)
	{
		// NOTE: indexes that are passed should not be on the edges!
		int count0 = 0;
		int count1 = 0;
		int count2 = 0;
		int count = 0;
		// int count3 = 0;
		// Precompute the row offsets
		int prev_row = index - ncols;
		int next_row = index + ncols;
		count0 = is_alive(&map[index - 1]) + is_alive(&map[index + 1]);
		count1 = is_alive(&map[prev_row]) + is_alive(&map[prev_row - 1]) + is_alive(&map[prev_row + 1]);
		count2 = is_alive(&map[next_row]) + is_alive(&map[next_row - 1]) + is_alive(&map[next_row + 1]);
		// count1 = is_alive(&map[index - ncols]) + is_alive(&map[index + ncols]);
		// count2 = is_alive(&map[index - ncols - 1]) + is_alive(&map[index - ncols + 1]);
		// count3 = is_alive(&map[index + ncols - 1]) + is_alive(&map[index + ncols + 1]);
		// count = count0+count1+count2+count3;
		return count;  
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

void ordered_evolution(unsigned char *restrict map, int ncols, int nrows)
{
	int i = 0;
	int alive_counter = 0;
    #ifdef _OPENMP
		#pragma omp parallel for schedule(static) firstprivate(i, alive_counter)
	#endif
	    	for (int row = 1; row < nrows-1; row++)
	    	{
	    	    for (int col = 0; col < ncols; col++)
	    	    {
	    	        i = row*ncols+ col;
	    	        alive_counter = count_alive_neighbours_multi(map, ncols, i);
	    	        map[i] = UPDATE_CELL(alive_counter);
	    	    }
	    	}
}


void shift_old_map(unsigned char *restrict map, const int ncols, const int nrows, const char shift)
{
	int i = 0;
	#ifdef _OPENMP
		#pragma omp for schedule(auto) private(i)
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

void static_evolution(unsigned char *restrict current, int ncols, int nrows, char shift){
    /*Performs a single step of the update of the map*/
	//int n_inner_rows = nrows -2;	
	// int left_border_counter = 0;
	// int right_border_counter = 0;
	int i = 0;
	shift_old_map(current, ncols, nrows, shift);	
	// int alive_counter = 0;

	#ifdef _OPENMP
		#pragma omp parallel
	#endif
	{
		#ifdef _OPENMP
			#pragma omp for schedule(static, 4) private(i)
		#endif	
		for(int row=1; row < nrows -1 ; row++){
			for(int col=1; col<ncols-1; col += CACHE_LINE_SIZE){
				for(int cc=col; cc<col+CACHE_LINE_SIZE && cc<ncols; cc++){
					i = row*ncols+ cc;
					int alive_counter = count_alive_neighbours_multi(current, ncols, i);
					current[i] += UPDATE_CELL(alive_counter);
				}
			}
			int left_border_counter = 0;
			int right_border_counter = 0;
			// for(int col=1; col < ncols-1; col++){
			// 	i = row*ncols+ col;
			// 	int alive_counter = count_alive_neighbours_multi(current, ncols, i);
			// 	// private_row[col] += count_alive_neighbours_multi(current, ncols, i);
			// 	current[i] += UPDATE_CELL(alive_counter);
			// 	// current[i] += UPDATE_CELL(3);
			// }
		
			/*Count alive neighbours for left and right
  			 border elements */

			i = row*ncols;
			left_border_counter = 0;
			right_border_counter = 0;

			left_border_counter += is_alive(&current[i-1]);	
			left_border_counter += is_alive(&current[i+1]);
			left_border_counter += is_alive(&current[i+ncols-1]);
			left_border_counter += is_alive(&current[i+ncols+1]);
			left_border_counter += is_alive(&current[i+ncols]);
			left_border_counter += is_alive(&current[i+2*ncols-1]);	
			left_border_counter += is_alive(&current[i-ncols]);
			left_border_counter += is_alive(&current[i-ncols+1]);
			current[i] += UPDATE_CELL(left_border_counter);

			i += ncols -1;
			right_border_counter += is_alive(&current[i+1]);	
			right_border_counter += is_alive(&current[i-1]);
			right_border_counter += is_alive(&current[i+ncols-1]);
			right_border_counter += is_alive(&current[i+ncols]);	
			right_border_counter += is_alive(&current[i-2*ncols+1]);	
			right_border_counter += is_alive(&current[i-ncols+1]);	
			right_border_counter += is_alive(&current[i-ncols-1]);	
			right_border_counter += is_alive(&current[i-ncols]);	
		
			current[i] += UPDATE_CELL(right_border_counter);
		}
	}
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

//void split_initial_matrix(unsigned char* restrict map1, const Env env, const int start_indices[]){
///* Allocates appropriate memory in each MPI process and
// * Splits initial matrix bewteen MPI processes
// */
//
//	//Allocate space for local maps
//	unsigned char *sub_map = (unsigned char*)malloc(env.my_process_rows*env.k*sizeof(unsigned char));
//	unsigned char *sub_map_copy = (unsigned char*)malloc(env.my_process_rows*env.k*sizeof(unsigned char));
//		
//	if (sub_map == NULL || sub_map_copy == NULL){
//		printf("Process %d error: Could not allocate memory for local maps\n", env.process_rank);
//		exit(1);
//	}
//
//	/* Initialize local sub matrices with OpenMP, to warm up the data properly*/
//	#pragma omp parallel for
//	for (int i=0; i<env.my_process_rows*env.k; i++){
//		sub_map[i] = 0;	
//		sub_map_copy[i]=0;
//	}	
//	
//	if (env.process_rank == 0){
//		/* Sending initial map to every MPI process */
//		for (int rank = 1; rank < env.size_of_cluster; rank++) {
//			MPI_Send(map1 + start_indices[rank]*env.k, env.rows_per_processor[rank]*env.k, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD);
//		}
//	} else {
//		MPI_Recv(sub_map, env.my_process_rows * env.k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//	}
//		/* Each process now has its copy of the submap */
//
//}
