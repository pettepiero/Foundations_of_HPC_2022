#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include <omp.h>


unsigned char *generate_blinker(unsigned char *restrict map, char fileName[], const int ncols, const int nrows) {
    	// Blinker
    	map[((int)(nrows-1)/2 -1)* ncols + (int)((ncols-1)/2)] = 255;
    	map[((int)(nrows-1)/2) * ncols + (int)((ncols-1)/2)] = 255;
    	map[((int)(nrows-1)/2 +1)* ncols + (int)((ncols-1)/2)] = 255;

    	write_pgm_image(map, MAXVAL, ncols, nrows, fileName);
	printf("\n\n");    
	printf("PGM file created: %s with dimensions %d x %d\n", fileName, nrows, ncols);

    return map;
}


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

void update_horizontal_edges(unsigned char *restrict map, const int ncols, const int nrows)
{
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

unsigned char *generate_map(unsigned char *restrict map, char* fname, const float probability, const int ncols, const int nrows, const int seed){
    	if(seed == 0)
        	srand(time(0));
    	else
		srand(seed);

    // populate pixels
	for (int i=1; i<nrows-1; i++){
        	for(int j = 0; j <ncols; j++){
            		float random = (float)rand()/ RAND_MAX;
            		if (random  < probability)
                		map[i*ncols + j] = 255;
            		else
                		map[i*ncols +j] = 0;
            	}
    	}

/* update boundary conditions (first and last rows)*/
	update_horizontal_edges(map, ncols, nrows);
    	write_pgm_image(map, MAXVAL, ncols, nrows, fname);
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

int count_alive_neighbours(unsigned char *restrict map, const int ncols, const int index)
{
	// NOTE: indexes that are passed should not be on the edges!
	if (index >= ncols){
		int count0 = 0;
		int count1 = 0;
		int count2 = 0;
		int count3 = 0;
		int count = 0;
		int neighbours[] = {index-1, index+1, index-ncols, index+ncols, index-ncols-1, index-ncols+1, index+ncols-1, index+ncols+1};

		for (int i = 0; i < 8; i+=4){
			    count0 += map[neighbours[i]]; 
			    count1 += map[neighbours[i+1]]; 
			    count2 += map[neighbours[i+2]]; 
			    count3 += map[neighbours[i+3]]; 
		}
		count = count0+count1+count2+count3;
		return count/255;  /*Will be optimized by -O2 option of gcc into multiplication and shift */
	}
	else {
		printf("Error in count_alive_neighbours\n");
		printf("index < ncols\n");
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
        return MAXVAL;
}



void ordered_evolution(unsigned char *restrict map, int ncols, int nrows)
{
    	for (int row = 1; row < nrows-1; row++)
    	{
    	    for (int col = 0; col < ncols; col++)
    	    {
    	        int i = row*ncols+ col;
    	        int alive_counter = count_alive_neighbours(map, ncols, i);
    	        map[i] = update_cell(alive_counter);
    	    }
    	}
}


void static_evolution(unsigned char *restrict current, unsigned char *restrict new, int ncols, int nrows){
	int n_inner_rows = nrows -2;	
    /*Performs a single step of the update of the map*/
   
	int left_border_counter = 0;
	int right_border_counter = 0;
	int i = 0;
	int chunk_size = 64*2;

	#if defined(_OPENMP)
	#pragma omp parallel firstprivate(left_border_counter, right_border_counter, i) 
	{    
		#pragma omp for schedule(static)

		for(int row=1; row <= n_inner_rows; row++){
			for(int col=1; col < ncols-1; col++){
				i = row*ncols+ col;
				int alive_counter = count_alive_neighbours(current, ncols, i);
				new[i] = update_cell(alive_counter);
			}
			/*Count alive neighbours for left and right
  			 border elements */
			i = row*ncols;
			left_border_counter = 0;
			right_border_counter = 0;
			/* left neighbours of cell on left edge */
			left_border_counter += current[i-1];	
			left_border_counter += current[i+ncols-1];
			left_border_counter += current[i+2*ncols-1];
			/* top and bottom neighbours of cell on left edge */	
			left_border_counter += current[i-ncols];
			left_border_counter += current[i+ncols];
			/* right neighbours of cell on left edge */
			left_border_counter += current[i-ncols+1];
			left_border_counter += current[i+1];
			left_border_counter += current[i+ncols+1];
			new[i] = update_cell(left_border_counter/255);

			i += ncols -1;
			/* right neighbours of cell on right edge */
			right_border_counter += current[i-2*ncols+1];	
			right_border_counter += current[i-ncols+1];	
			right_border_counter += current[i+1];	
			/* top and bottom neighbours of cell on right edge */
			right_border_counter += current[i-ncols];	
			right_border_counter += current[i+ncols];	
			/* left neighbouts of cell on right edge */
			right_border_counter += current[i-ncols-1];	
			right_border_counter += current[i-1];
			right_border_counter += current[i+ncols-1];
			new[i] = update_cell(right_border_counter/255);
		}
	}
	#else

	int alive_counter = 0;
	int left_border_counter = 0;
	int right_border_counter = 0;
	int i = 0;
		/* Inner matrix loop */
	for(int row=1; row <= n_inner_rows; row++){
		for(int col=1; col < ncols-1; col++){
			i = row*ncols + col;
			alive_counter = count_alive_neighbours(current, ncols, i);
			new[i] = update_cell(alive_counter);
		}	

		/*Count alive neighbours for left and right
  		 border elements */
		printf("Border element %d=(%d, %d)\n", i, row, col);
		i = row*ncols;
		left_border_counter = 0;
		right_border_counter = 0;
		/* left neighbours of cell on left edge */
		left_border_counter += current[i-1];	
		left_border_counter += current[i+ncols-1];	
		left_border_counter += current[i+2*ncols-1];
		/* top and bottom neighbours of cell on left edge */	
		left_border_counter += current[i-ncols];	
		left_border_counter += current[i+ncols];	
		/* right neighbours of cell on left edge */
		left_border_counter += current[i-ncols+1];	
		left_border_counter += current[i+1];
		left_border_counter += current[i+ncols+1];
		new[i] = update_cell(left_border_counter);

		i += ncols -1;
		/* right neighbours of cell on right edge */
		right_border_counter += current[i-2*ncols+1];	
		right_border_counter += current[i-ncols+1];	
		right_border_counter += current[i+1];	
		/* top and bottom neighbours of cell on right edge */
		right_border_counter += current[i-ncols];	
		right_border_counter += current[i+ncols];	
		/* left neighbouts of cell on right edge */
		right_border_counter += current[i-ncols-1];	
		right_border_counter += current[i-1];
		right_border_counter += current[i+ncols-1];
		new[i] = update_cell(right_border_counter);
	}
	#endif
	memcpy(current, new, ncols*nrows*sizeof(unsigned char));
}
