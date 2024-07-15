#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include <omp.h>


#ifdef BLINKER
unsigned char *generate_blinker(unsigned char *restrict map, char fileName[], int size){
    // Blinker
    map[(int)(size)/2 * size + (int)(size /2)] = 255;
    map[((int)(size)/2 +1) * size + (int)(size /2)] = 255;
    map[((int)(size)/2 +2)* size + (int)(size /2)] = 255;

    write_pgm_image(map, maxval, size, size, fileName);
    printf("PGM file created: %s\n", fileName);
    return map;
}
#endif


void init_to_zero(unsigned char *restrict map1, const int k)
{
    #if defined(_OPENMP)
        // Touching the maps in the different threads,
        // so that each cache will be warmed-up appropriately
        #pragma omp parallel
        {
            for (int i=0; i<k; i++){
                for(int j = 0; j <k; j++){
                    map1[i*k +j] = 0;
                }
            }
        }
        #else
            for (int i=0; i<k; i++){
                for(int j = 0; j<k; j++)
                    map1[i*k +j] = 0;
            }
    #endif
}

void update_horizontal_edges(unsigned char *restrict map, const int n_cols, const int n_inner_rows)
{
    // update top and bottom edges
    for (int i=0; i<n_cols; i++)
    {
        map[i] = map[i+n_cols*n_inner_rows];
        map[i+n_cols*(n_inner_rows+1)] = map[i+n_cols];
    }
}

void print_map(unsigned char *restrict map, const int k)
{
    for (int i=0; i<k; i++){
        for(int j = 0; j <k; j++){
            printf("%d\t", map[i*k +j]);
        }
        printf("\n");
    }
}

void print_map_to_file(unsigned char *restrict map, const int k, const char fileName[])
{
    FILE *f = fopen(fileName, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    fclose(f);

    f = fopen(fileName, "a");
    for (int i=0; i<k; i++){
        for(int j = 0; j <k; j++){
            fprintf(f, "%d\t", map[i*k +j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);

}

unsigned char *generate_map(unsigned char *restrict map, const char fileName[], const float probability, const int k, const int seed){

    #ifdef DEBUG
    printf("Generating random map:\n");
    #endif

    if(seed == 0)
        srand(time(0));
    else
        srand(seed);

    // populate pixels
    for (int i=0; i<k; i++){
        for(int j = 0; j <k; j++){
            float random = (float)rand()/ RAND_MAX;
            if (random  < probability)
                map[i*k + j] = 255;
            else
                map[i*k +j] = 0;
            }
    }

    write_pgm_image(map, maxval, k, k, fileName);
    printf("PGM file created: %s\n", fileName);
    return map;
}

// A cell that did not change at the last time step, and none of whose neighbors changed,
// is guaranteed not to change at the current time step as well, so a program that keeps 
// track of which areas are active can save time by not updating inactive zones.
void get_active_zones()
{}


// Defenitely the wrong way to do it but it's a start
//IDEA: Use a lookup table for the neighbours ? 
//IDEA: would it be faster to calculate neighbours for more than one position, therefore reducing #function calls?

int count_alive_neighbours(unsigned char *restrict map, const int n_cols, const int index)
{
	// NOTE: indexes that are passed should not be on the edges!
	if (index > n_cols){
		int count = 0;
		int neighbours[] = {index-1, index+1, index-n_cols, index+n_cols, index-n_cols-1, index-n_cols+1, index+n_cols-1, index+n_cols+1};

	    // #ifdef DEBUG
	    // printf("Neighbours of %d: \n", index);
	    // for (int i=0; i<8; i++)
	    // {
	    //     printf("%d=%d \n", neighbours[i], map[neighbours[i]]);
	    // }
	    // #endif

		for (int i = 0; i < 8; i++)
			if (map[neighbours[i]] == 255)
			    count++;

		return count;
	}
	else {
		printf("Error in count_alive_neighbours\n");
		printf("index < n_cols\n");
		exit(1);
	}

}

// I believe the compiler will optimize this with a ternary operation.
// I have left it so for the sake of readability
char update_cell(const int alive_neighbours)
{
    if(alive_neighbours < 2 || alive_neighbours > 3)
        return 0;
    else
        return 255;
}

void n_colsordered_evolution(unsigned char *restrict map, int n_cols, int n_rows)
{
    for (int row = 1; row < n_rows-1; row++)
    {
        for (int col = 1; col < n_cols-1; col++)
        {
            int i = row*n_cols+ col;
            int alive_counter = count_alive_neighbours(map, n_cols, i);
            map[i] = update_cell(alive_counter);
        }
    }
}


void edges_static_evolution(unsigned char *restrict current, unsigned char *restrict new, unsigned char left_col[], unsigned char right_col[], int n_inner_rows, int n_cols){
	int left_counter = 0;
	int right_counter = 0;
	for(int row=1; row < n_inner_rows-1; row++){
		left_counter += left_col[row-1];	
		left_counter += left_col[row];	
		left_counter += left_col[row+1];	
		left_counter += current[(row-1)*n_cols+1];	
		left_counter += current[(row-1)*n_cols];	
		left_counter += current[row*n_cols+1];	
		left_counter += current[(row+1)*n_cols];
		left_counter += current[(row+1)*n_cols+1];
		new[row*n_cols] = update_cell(left_counter);

		right_counter += right_col[row-1];	
		right_counter += right_col[row];	
		right_counter += right_col[row+1];	
		right_counter += current[(row-1)*n_cols+n_cols-1];	
		right_counter += current[(row-1)*n_cols+n_cols-2];	
		right_counter += current[row*n_cols+n_cols-2];	
		right_counter += current[(row+1)*n_cols+n_cols-1];
		right_counter += current[(row+1)*n_cols+n_cols-2];
		new[row*n_cols+n_cols-1] = update_cell(right_counter);
	} 
}

void static_evolution(unsigned char *restrict current, unsigned char *restrict new, unsigned char left_col[], unsigned char right_col[], int n_inner_rows, int n_cols)
{
    /*Performs a single step of the update of the map*/
   
	#if defined(_OPENMP)
	#pragma omp parallel {    
		int i = 0;
		#pragma omp for collapse(2)
		for(int row=1; row < n_inner_rows-1; row++)
		    for(int col=1; col < n_cols-1; col++)    
		{
		    i = row*size + col;
		    int alive_counter = count_alive_neighbours(current, size, i);
		    new[i] = update_cell(alive_counter);
		}
	}
	#else

	int alive_counter = 0;
	int left_counter = 0;
	int right_counter = 0;
	int i = 0;
	/*for(int row=1; row < n_inner_rows-1; row++)
	    for(int col=0; col < n_cols; col++)    
	{
	    i = row*n_cols + col;
	    alive_counter = count_alive_neighbours(current, n_cols, i);
	    new[i] = update_cell(alive_counter);
	}*/

	for(int row=1; row < n_inner_rows-1; row++){
		for(int col=1; col < n_cols-1; col++){
		/*Count alive neighbours easily from matrix */
			i = row*n_cols + col;
			alive_counter = count_alive_neighbours(current, n_cols, i);
			new[i] = update_cell(alive_counter);
			
		}	
		/*Count alive neighbours for left and right
 * 		 border elements */
		left_counter = 0;
		right_counter = 0;
		left_counter += left_col[row-1];	
		left_counter += left_col[row];	
		left_counter += left_col[row+1];	
		left_counter += current[(row-1)*n_cols+1];	
		left_counter += current[(row-1)*n_cols];	
		left_counter += current[row*n_cols+1];	
		left_counter += current[(row+1)*n_cols];
		left_counter += current[(row+1)*n_cols+1];
		new[row*n_cols] = update_cell(left_counter);

		right_counter += right_col[row-1];	
		right_counter += right_col[row];	
		right_counter += right_col[row+1];	
		right_counter += current[(row-1)*n_cols+n_cols-1];	
		right_counter += current[(row-1)*n_cols+n_cols-2];	
		right_counter += current[row*n_cols+n_cols-2];	
		right_counter += current[(row+1)*n_cols+n_cols-1];
		right_counter += current[(row+1)*n_cols+n_cols-2];
		new[row*n_cols+n_cols-1] = update_cell(right_counter);
	}
	#endif

	update_horizontal_edges(new, n_cols, n_inner_rows);

	memcpy(current, new, n_cols*(n_inner_rows)+2*sizeof(unsigned char));
}
