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
// #include <mpi.h>

#if !defined(_OPENMP)
#warning "Run "make openmp" to enable OpenMP."
#endif

int   action = 0;
int   k      = K_DFLT + 2;
int   e      = STATIC;
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
    // MPI_Init(&argc, &argv);
    
    // int size_of_cluster;
    // int process_rank;

    // MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
    // MPI_Comm_rank(MPI_COMM_wORLD, &process_rank);

    // if(process_rank == 0)
    // {
    //     printf("Rank: %d |\tSize of cluster: %d\n", process_rank, size_of_cluster);
    //     int n_lines_per_process = k / size_of_cluster;
    //     printf("Rank: %d |\tNumber of lines per process: %d/%d=%d\n", k, size_of_cluster, process_rank, n_lines_per_process);
    // }


    #ifndef _OPENMP
	printf("\nExecuting without OPENMP in serial mode.\n\n");
    #endif
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
        e = atoi(optarg); 
        if(e == STATIC)
            printf("Using static evolution\n");
        else if(e == ORDERED)
            printf("Using ordered evolution\n");
        else{
            printf("Unknown evolution type\n");
            exit(1);
        }
        break;

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
    
    void *map1 = NULL;
    void *map2 = NULL;
    unsigned char *map1_char = NULL;
    unsigned char *map2_char = NULL;

    /* Determines if map has to be initialized or read from file */
    if(action == RUN){
        printf("******************************\nRunning a playground\n******************************\n");
        char file[] = "images/blinker.pgm";
        printf("Reading map from %s\n", file);
        read_pgm_image(&map1, &maxval, &k, &k, file);
        write_pgm_image(map1, maxval, k, k, "images/copy_of_image.pgm");
        printf("Read map from %s\n", file);
    }
    else if(action == INIT){
        map1 = (unsigned char*)malloc(k*k*sizeof(unsigned char));
        if (map1 == NULL)
        {
            printf("Error: Could not allocate memory for map1\n");
            exit(1);
        }
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
                "r - Run a world\n"
                "i - Initialize a world\n"
                "Evolution types:\n"
                "e 0 - Ordered evolution\n"
                "e 1 - Static evolution\n\n");
        exit(1);
    }

    map1_char = (unsigned char*)map1;

    if (e == STATIC){
        map2 = (unsigned char*)malloc(k*k*sizeof(unsigned char));
        if (map2 == NULL)
        {
            printf("Error: Could not allocate memory for map2\n");
            exit(1);
        }
        map2_char = (unsigned char*)map2;
        memcpy(map2_char, map1_char, k*k*sizeof(char));
    } else if (e == ORDERED)
        init_to_zero(map1_char, k);

    #if defined(_OPENMP)
    #pragma omp parallel 
    {
        #pragma omp master 
        {
            int nthreads = omp_get_num_threads();
            printf("Going to use %d threads\n", nthreads );
        }
    }
    #endif

    #ifdef PROFILING
        printf("***************************************\n");
        printf("In profiling mode, not generating images\n");
        printf("***************************************\n");
        double tstart  = CPU_TIME;
    #endif

    // for(int i = 0; i < N_STEPS; i++)
    // {
    //     #ifdef STATIC
    //         update_map(map1, map2, k);
    //     #else
    //         update_map(map1, map1, k);
    //     #endif
        
    //     #ifndef PROFILING
    //         sprintf(fname, "images/snapshots/snapshot%d.pgm", i);
    //         write_pgm_image(map1, maxval, k, k, fname);
    //     #endif

    // }

    #ifdef PROFILING
        double tend = CPU_TIME;
        double ex_time = tend-tstart;
        printf("\n\n Execution time is %f\n\n", ex_time);
    #endif
    
    free(map1);
    printf("Address of map1: %p\n", map1);
    printf("Address of map1_char: %p\n", map1_char);
    if (e == STATIC)
        free(map2);
        
    // MPI_Finalize();    
    return 0;
}
