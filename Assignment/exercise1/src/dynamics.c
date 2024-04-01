#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"

#ifdef BLINKER
unsigned char *generate_blinker(unsigned char* map, char fileName[], int size){
    // Blinker
    map[(int)(NROWS)/2 * NCOLS + (int)(NCOLS /2)] = 255;
    map[((int)(NROWS)/2 +1) * NCOLS + (int)(NCOLS /2)] = 255;
    map[((int)(NROWS)/2 +2)* NCOLS + (int)(NCOLS /2)] = 255;

    write_pgm_image(map, maxval, size, size, fileName);
    printf("PGM file created: %s\n", fileName);
    return map;
}
#endif

unsigned char *generate_map(unsigned char* map, char fileName[], float probability, int size){
    
    srand(time(0)); 
    // Populate pixels
    for (int i = 0; i < NROWS; i++) {
        for (int j = 0; j < NCOLS; j++) {
            if ((float)rand() / RAND_MAX < probability) {
                map[i * NCOLS + j] = 255;
            } else {
                map[i * NCOLS + j] = 0;
            }
        }
    }

    write_pgm_image(map, maxval, size, size, fileName);
    printf("PGM file created: %s\n", fileName);
    return map;
}

// A cell that did not change at the last time step, and none of whose neighbors changed,
// is guaranteed not to change at the current time step as well, so a program that keeps 
// track of which areas are active can save time by not updating inactive zones.
void get_active_zones()
{}


// This function defenitely needs to be optimized
void get_wrapped_neighbors(const int num_rows, const int num_cols, int index, int neighbors[]) {
    int relative_positions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},          {0, 1},
        {1, -1},  {1, 0}, {1, 1}
    };
    
    int row = index / num_cols;
    int col = index % num_cols;

    for (int i = 0; i < 8; i++) {
        int row_shift = relative_positions[i][0];
        int col_shift = relative_positions[i][1];
        
        // Calculate the wrapped indices using modular arithmetic
        int wrapped_row = (row + row_shift + num_rows) % num_rows;
        int wrapped_col = (col + col_shift + num_cols) % num_cols;
        
        int index = wrapped_row * num_cols + wrapped_col;
        
        neighbors[i] = index;
    }
}

// Defenitely the wrong way to do it but it's a start
int count_alive_neighbours(unsigned char *map, int size, int index)
{
    int count = 0;
    int neighbours[] = {index-1, index+1, index-size, index+size, index-size-1, index-size+1, index+size-1, index+size+1};

    //If index is on the edges of the map, then neighbours needs to be adjusted
    if (index < size || index >= size*(size-1) || index % size == 0 || (index+1) % size == 0)
    {
        get_wrapped_neighbors(size, size, index, neighbours); 
    }

    // #ifdef DEBUG
    // printf("Neighbours of %d: ", index);
    // for (int i=0; i<8; i++)
    // {
    //     printf("%d=%d ", neighbours[i], map[neighbours[i]]);
    // }
    // #endif

    for (int i = 0; i < 8; i++)
    {
        if (map[neighbours[i]] == 255)
        {
            count++;
        }
    }

    return count;
}

char update_cell(int alive_neighbours)
{
    if(alive_neighbours < 2 || alive_neighbours > 3)
    {
        // printf("Setting cell to 0\n");
        return 0;
    }
    else
    {
        return 255;
        // printf("Setting cell to 1\n");
    }
}

// Performs a single step of the update of the map
void update_map(unsigned char *current, unsigned char *new, int size)
{
    #ifdef DEBUG
    printf("Inside update_map\n");
    #endif

    // loop over elements
    #ifdef DEBUG
    printf("Inside update_map\n");
    for(int i=0; i < 20; i++)
    #endif
    #ifndef DEBUG
    for(int i=0; i < size*size; i++)
    #endif    
    {
        // check neighbours
        int alive_counter = count_alive_neighbours(current, size, i);
        // update element
        new[i] = update_cell(alive_counter);
        #ifdef DEBUG
        printf("Cell %d, counted %d alive cells, new[%d] = %d\n", i, alive_counter, i , new[i]);
        #endif
    }

    memcpy(current, new, size*size*sizeof(char));

    #ifdef DEBUG
    printf("Updated map\n");
    printf("Printing first 100 elements\n");
    for(int i=0; i < 100; i++)
    {
        printf("%d ", current[i]);
    }
    #endif
}