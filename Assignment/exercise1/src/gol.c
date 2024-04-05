/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */

#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include "dynamics.h"

int   action = 0;
int   k      = K_DFLT;
int   e      = ORDERED;
int   n      = 10000;
int   s      = 1;
char *fname  = NULL;
int maxval = 255; //255 -> white, 0 -> black

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

    // Allocate memory for map and copy of map
    
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

    // Determines if map has to be initialized or read from file
    if(action == RUN){
        printf("******************************\nRunning a playground\n******************************\n");
        char file[] = "images/blinker.pgm";
        map1 = (unsigned char*)malloc(k*k*sizeof(char));
        printf("Reading map from %s\n", file);
        read_pgm_image(&map1, &maxval, &k, &k, file);
        write_pgm_image(map1, maxval, k, k, "images/copy_of_image.pgm");
        memcpy(map2, map1, k);
        printf("Read map from %s\n", file);
    } 
    else if(action == INIT){
        printf("******************************\nInitializing a playground\n******************************\n");
        #ifdef BLINKER
        printf("Generating blinker\n");
        map1 = (unsigned char*)calloc(k*k, sizeof(unsigned char));
        generate_blinker(map1, "images/blinker.pgm", k);
        printf("Blinker created\n");
        break;
        #endif
        #ifndef BLINKER
        generate_map(map1, "images/initial_map.pgm", 0.05, k);
        #endif
        #ifdef DEBUG
        printf("Printing first 100 elements after create_map()\n");
        for(int i=0; i < 100; i++)
        {
            printf("%d ", ((unsigned char *)map1)[i]);
        }
        printf("\n");
        #endif
        printf("Address of 'map1' = ");
        printf("  %p\n", &map1);
    } else {
        printf("No action specified\n");
        printf("Possible actions:\n"
                "i - Initialize a world\n"
                "r - Run a world\n");
        exit(1);
    }

    // Copy map2 into map1 and perform N_STEPS updates
    memcpy(map2, map1, k*k*sizeof(char));

    char fname[100];

    for(int i = 0; i < N_STEPS; i++)
    {
        sprintf(fname, "images/snapshots/snapshot%d.pgm", i);
        #ifdef DEBUG
        printf("Step %d\n", i);
        #endif

        update_map(map1, map2, k);

        printf("\n\n Trying to execute \'write_pgm_image()'\n\n");
        write_pgm_image(map1, maxval, k, k, fname);
    }
    
    free(map1);
    free(map2);
    return 0;
}