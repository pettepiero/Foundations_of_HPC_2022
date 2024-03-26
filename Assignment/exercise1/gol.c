/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */


#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"

#define NROWS 100
#define NCOLS 100

#define INIT 1
#define RUN  2

#define K_DFLT 100

#define ORDERED 0
#define STATIC  1
#define N_STEPS 10

//#define DEBUG

char fname_deflt[] = "game_of_life.pgm";

int   action = 0;
int   k      = K_DFLT;
int   e      = ORDERED;
int   n      = 10000;
int   s      = 1;
char *fname  = NULL;
int maxval = 255; //255 -> white, 0 -> black

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

void create_map(unsigned char* ptr, int size)
{
    // The approach is to use a 1 dimensional array to represent the 2D map
    // and perform index calculations (cache locality)
    // The size is incremented by one because the edges of the map
    // are copies of the opposite edge. Therefore loops will be easier to write
    // and should always be in the inner matrix.

    generate_map(ptr, "initial_map.pgm", 0.2, size);
    //read_pgm_image(ptr, &maxval, &size, &size, "initial_map.pgm");
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

//To save memory, the storage can be reduced to one array plus two line buffers.
// One line buffer is used to calculate the successor state for a line, then the second 
// line buffer is used to calculate the successor state for the next line. The first buffer
// is then written to its line and freed to hold the successor state for the third line.

int main(int argc, char **argv)
{
    int action = 0;
    char *optstring = "irk:e:f:n:s:";

    int c;
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
      
        case 'i':
        action = INIT; break;
        
        case 'r':
        action = RUN; 
        break;
        
        case 'k':
        k = atoi(optarg); break;

        case 'e':
        e = atoi(optarg); break;

        case 'f':
        fname = (char*)malloc( sizeof(optarg)+1 );
        sprintf(fname, "%s", optarg );
        printf("Reading file: %s\n", fname);
        break;

        case 'n':
        n = atoi(optarg); break;

        case 's':
        s = atoi(optarg); break;

        default :
        printf("argument -%c not known\n", c ); break;
        }
    }

    if ( fname != NULL )
        free ( fname );

    void *map1 = (unsigned char*)malloc(k*k*sizeof(char));
    if (map1 == NULL)
    {
        printf("Error: Could not allocate memory for map1\n");
        exit(1);
    }
    void *map2 = (unsigned char*)malloc(k*k*sizeof(char));
    if (map2 == NULL)
    {
        printf("Error: Could not allocate memory for map2\n");
        exit(1);
    }

    switch(action)
    {
        case INIT:
        printf("******************************\nInitializing a playground\n******************************\n");
        create_map(map1, k);
        #ifdef DEBUG
        printf("Printing first 100 elements after create_map()\n");
        for(int i=0; i < 100; i++)
        {
            printf("%d ", ((unsigned char *)map1)[i]);
        }
        printf("\n");
        #endif
        memcpy(map2, map1, k*k*sizeof(char));
        char fname[15];
        for(int i = 0; i < N_STEPS; i++)
        {
            sprintf(fname, "snapshot%d.pgm", i);
            printf("Step %d\n", i);
            update_map(map1, map2, k);
            write_pgm_image(map1, maxval, k, k, fname);
        }
        update_map(map1, map2, k);
        break;

        case RUN:
        printf("******************************\nRunning a playground\n******************************\n");        break;
        char file[] = "image.pgm";
        map1 = (unsigned char*)malloc(k*k*sizeof(char));
        read_pgm_image(&map1, &maxval, &k, &k, file);
        memcpy(map2, map1, k);
        printf("Read map from %s\n", file);
        break;

        default:
        printf("No action specified\n");
        break;
    }     

    free(map1);
    free(map2);
    return 0;
}