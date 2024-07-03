#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <mpi.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include "dynamics.h"

int maxval = 255; //255 -> white, 0 -> black

int main(int argc, char **argv)
{
    int action = 0;
    int k = K_DFLT + 2;
    int e = ORDERED;
    int n = 10000;
    int s = 1;
    char *fname = NULL;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank ==0){
        printf("Communicator size = %d\n", size);
        printf("This rank is %d\n", rank);
    }

    char *optstring = "irk:e:f:n:s:";
    int c;
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
            case 'i':
                action = INIT;
                break;
            case 'r':
                action = RUN;
                break;
            case 'k':
                k = atoi(optarg);
                break;
            case 'e':
                e = atoi(optarg);
                break;
            case 'f':
                if (optarg != NULL && *optarg != '\0') {
                    fname = (char*)malloc(strlen(optarg) + 1);
                    if (fname==NULL){
                        printf("Error on allocating fname\n");
                        MPI_Finalize();
                        exit(1);
                    }
                    sprintf(fname, "%s", optarg);
                    if(rank == 0)
                        printf("Reading file: %s\n", fname);
                } else {
                   printf("Error: optarg null when checking file name\n");
                }
                break;
            case 'n':
                n = atoi(optarg);
                break;
            case 's':
                s = atoi(optarg);
                break;
            default:
                if(rank == 0)
                    printf("argument -%c not known\n", c);
                break;
        }
    }

    if (fname != NULL)
        free(fname);

    int local_k = k / size;
    unsigned char *local_map1 = (unsigned char*)malloc(local_k * k * sizeof(unsigned char));
    unsigned char *local_map2 = (unsigned char*)malloc(local_k * k * sizeof(unsigned char));


    if (local_map1 == NULL || local_map2 == NULL) {
        printf("\n\n\nError: Could not allocate memory for local maps\n\n\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    memset(local_map1, 0, local_k * k * sizeof(unsigned char));
    memset(local_map2, 0, local_k * k * sizeof(unsigned char));

    unsigned char *map1 = NULL;
    if (rank == 0) {
        map1 = (unsigned char*)malloc(k * k * sizeof(unsigned char));
        if(map1==NULL){
            printf("\n\n\nError: could not allocate map1 on rank 0\n\n\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }
        memset(map1, 0, k * k * sizeof(unsigned char));
    }

    if (action == RUN) {
        if (rank == 0) {
            printf("******************************\nRunning a playground\n******************************\n");
            char file[] = "images/blinker.pgm";
            printf("Reading map from %s\n", file);
            read_pgm_image((void**)&map1, &maxval, &k, &k, file);
            write_pgm_image(map1, maxval, k, k, "images/copy_of_image.pgm");
            printf("Read map from %s\n", file);
        }
        // Broadcast to all other processes in the communicator
        MPI_Bcast(map1, k * k, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    } else if (action == INIT) {
        if (rank == 0) {
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
                for(int i = 0; i < 100; i++) {
                    printf("%d ", ((unsigned char *)map1)[i]);
                }
                printf("\n");
            #endif
        }
        #ifdef DEBUG
            printf("Rank: %d, Address of map1 = %p\n", rank, &map1);
        #endif

        MPI_Bcast(map1, k * k, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
        #ifdef  DEBUG
            printf("Finished else-if on variable action and broadcasted for the first time\n");
        #endif
    } else {
        if (rank == 0) {
            printf("No action specified\n");
            printf("Possible actions:\n"
                   "r - Run a world\n");
        }
        MPI_Finalize();
        exit(1);
    }

    MPI_Scatter(map1, local_k * k, MPI_UNSIGNED_CHAR, local_map1, local_k * k, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    for(int step = 0; step < N_STEPS; step++) {
        update_map(local_map1, local_map2, k);

        // Gather the updated local maps to the root process
        MPI_Gather(local_map2, local_k * k, MPI_UNSIGNED_CHAR, map1, local_k * k, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            #ifndef PROFILING
                sprintf(fname, "images/snapshots/snapshot%d.pgm", step);
                write_pgm_image(map1, maxval, k, k, fname);
            #endif
        }

        // Swap the pointers
        unsigned char *temp = local_map1;
        local_map1 = local_map2;
        local_map2 = temp;
    }

    if (rank == 0) {
        free(map1);
    }
    free(local_map1);
    free(local_map2);

    MPI_Finalize();
    return 0;
}