/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */


#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include "read_write_pgm_image.h"

// A cell that did not change at the last time step, and none of whose neighbors changed,
// is guaranteed not to change at the current time step as well, so a program that keeps 
// track of which areas are active can save time by not updating inactive zones.
void get_active_zones()
{}

void* create_map(int size)
/*
 *
 */
{
    void *ptr;
    // The approach is to use a 1 dimensional array to represent the 2D map
    // and perform index calculations (cache locality)
    ptr = (char*)calloc(size*size, sizeof(char));
    if (ptr == NULL)
    {
        printf("Error: Could not allocate memory for the map\n");
        exit(1);
    }
    printf("Created map\n");
    return ptr;
}


//To save memory, the storage can be reduced to one array plus two line buffers.
// One line buffer is used to calculate the successor state for a line, then the second 
// line buffer is used to calculate the successor state for the next line. The first buffer
// is then written to its line and freed to hold the successor state for the third line.

#define NROWS 10
#define NCOLS 10

#define INIT 1
#define RUN  2

#define K_DFLT 100

#define ORDERED 0
#define STATIC  1


char fname_deflt[] = "game_of_life.pgm";

int   action = 0;
int   k      = K_DFLT;
int   e      = ORDERED;
int   n      = 10000;
int   s      = 1;
char *fname  = NULL;
int maxval = 0;

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

    void *map;

    switch(action){
        case INIT:
        printf("******************************\nInitializing a playground\n******************************\n");
        void *map = create_map(k);
        break;

        case RUN:
        printf("******************************\nRunning a playground\n******************************\n");        break;
        char file[] = "image.pgm";
        read_pgm_image(&map, &maxval, &k, &k, file);
        printf("Read map from %s\n", file);
        break;

        default:
        printf("No action specified\n");
        break;
    }     



    return 0;
}