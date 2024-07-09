/* Piero Pettenà - UNITS, Foundations of High Performance Computing - Game of Life */


#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include "dynamics.h"
#include <omp.h>
#include <mpi.h>
#if !defined(_OPENMP)
#warning "Run "make openmp" to enable OpenMP."
#endif

int   action = 0;
int   k      = K_DFLT + 2;
int   e      = ORDERED;
int   n      = 10000;
int   s      = 1;
char *fname  = NULL;
int maxval = 255; //255 -> white, 0 -> black

//IDEA:
//To save memory, the storage can be reduced to one array plus two line buffers.
// One line buffer is used to calculate the successor state for a line, then the second 
// line buffer is used to calculate the successor state for the next line. The first buffer
// is then written to its line and freed to hold the successor state for the third line.

int main(int argc, char **argv)
{
	int rank, comm_size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &comm_size);
	
	if ( rank == 0){
		printf("Rank: %d |\t Splitting %d lines in %d processors\n", rank, k, comm_size);
		int n_lines_per_proc = k / comm_size;
		printf("n_lines_per_proc = k / comm_size = %d / %d = %d", k, comm_size, n_lines_per_proc);
	}

    action = 0;
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
        fname = (char*)malloc( strlen(optarg)+1 );
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
    
    void *map1 = (unsigned char*)malloc(k*k*sizeof(unsigned char));
    if (map1 == NULL)
    {
        printf("Error: Could not allocate memory for map1\n");
        exit(1);
    }
    void *map2 = (unsigned char*)malloc(k*k*sizeof(unsigned char));
    if (map2 == NULL)
    {
        printf("Error: Could not allocate memory for map2\n");
        exit(1);
    }

    unsigned char *map1_char = (unsigned char*)map1;
    unsigned char *map2_char = (unsigned char*)map2;

    #if defined(_OPENMP)
    // Touching the maps in the different threads,
    // so that each cache will be warmed-up appropriately
    #pragma omp parallel
    {
        for (int i=0; i<k; i++)
        {
            for(int j = 0; j <k; j++)
            {
                map1_char[i*k +j] = 0;
                map2_char[i*k +j] = 0;
            }
        }
    }
    #else
        for (int i=0; i<k; i++)
    {
        for(int j = 0; j <k; j++)
        {
            map1_char[i*k +j] = 0;
            map2_char[i*k +j] = 0;
        }
    }
    #endif

    // Determines if map has to be initialized or read from file
    if(action == RUN){
        printf("******************************\nRunning a playground\n******************************\n");
        char file[] = "images/blinker.pgm";
        printf("Reading map from %s\n", file);
        read_pgm_image(&map1, &maxval, &k, &k, file);
        write_pgm_image(map1, maxval, k, k, "images/copy_of_image.pgm");
        memcpy(map2, map1, k);
        printf("Read map from %s\n", file);
    } 
    else if(action == INIT){
        #ifdef DEBUG
            printf("******************************\nInitializing a playground\n******************************\n");
        #endif
        #ifdef BLINKER
            #ifdef DEBUG
                printf("Generating blinker\n");
            #endif
            generate_blinker(map1, "images/blinker.pgm", k);
        #endif

        #ifndef BLINKER
            generate_map(map1, "images/initial_map.pgm", 0.10, k, 0);
        #endif
        #ifdef DEBUG
            printf("Printing first 100 elements after create_map()\n");
            for(int i=0; i < 100; i++)
            {
                printf("%d ", ((unsigned char *)map1)[i]);
            }
            printf("\n");
        #endif
    } else {
        printf("No action specified\n");
        printf("Possible actions:\n"
                "r - Run a world\n");
        exit(1);
    }

    // Copy map2 into map1 and perform N_STEPS updates
    memcpy(map2, map1, k*k*sizeof(char));

    #if defined(_OPENMP)
    #pragma omp parallel 
    {
        #pragma omp master 
        {
            int nthreads = omp_get_num_threads();
            printf("Going to use %d threads\n", nthreads );
        }
    }
    #else
    printf("Serial code, OpenMP disabled.\n");
    #endif

    #ifdef PROFILING
        printf("***************************************\n");
        printf("In profiling mode, not generating images\n");
        printf("***************************************\n");
        double tstart  = CPU_TIME;
    #endif

    for(int i = 0; i < N_STEPS; i++)
    {
	printf("Step %d\n", i);
        update_map(map1, map2, k);
        
        #ifndef PROFILING
            sprintf(fname, "images/snapshots/snapshot%d.pgm", i);
            write_pgm_image(map1, maxval, k, k, fname);
        #endif

    }

    #ifdef PROFILING
        double tend = CPU_TIME;
        double ex_time = tend-tstart;
        printf("\n\n Execution time is %f\n\n", ex_time);
    #endif
    
    free(map1);
    free(map2);
        
  MPI_Finalize();
    return 0;
}
