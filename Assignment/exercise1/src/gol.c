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
int   k      = K_DFLT;
int   k_boundaries = K_DFLT + 2;
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
    MPI_Init(&argc, &argv);
    
    int size_of_cluster;
    int process_rank;
    int n_lines_per_process;
    //int n_lines_of_last_process = 0;
    int rows_to_send = 0;
    //int last_rows_to_send = 0;
    void *map1 = NULL;
    void *map2 = NULL;
    unsigned char *map1_char = NULL;
    unsigned char *map2_char = NULL;
    unsigned char *local_map1 = NULL;
    unsigned char *local_map2 = NULL;
    char file[] = "images/blinker.pgm";

    MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    
    /* Things that only process 0 has to do:
        1. Parse command line arguments
        2. Read or generate matrix
        3. Calculate the lines per process*/
    if(process_rank == 0){

        command_line_parser(&action, &k, &k_boundaries, &e, &fname, &n, &s, argc, argv);
        
        printf("Rank: %d |\tSize of cluster: %d\n", process_rank, size_of_cluster);
        n_lines_per_process = k_boundaries / size_of_cluster;
        
        /*NOTE: deal with the case where k%size_of_cluster !=0 */
        // if (k%size_of_cluster != 0)
        //     n_lines_of_last_process = n_lines_per_process + k%size_of_cluster;

        rows_to_send = n_lines_per_process + 2; // Each process gets an extra row above and below
        // last_rows_to_send = n_lines_of_last_process + 2;

        printf("Rank: %d |\tNumber of lines per process: %d/%d=%d\n", k_boundaries, size_of_cluster, process_rank, n_lines_per_process);
        #ifndef _OPENMP
            printf("\nExecuting without OPENMP in serial mode.\n");
        #endif
        }

        if((action != RUN) && (action != INIT)){
            printf("No action specified\n");
            printf("Possible actions:\n"
                    "r - Run a world\n"
                    "i - Initialize a world\n"
                    "Evolution types:\n"
                    "e 0 - Ordered evolution\n"
                    "e 1 - Static evolution\n\n");
            exit(1);
        }

        set_up_map_variable(action, e, k_boundaries, map1, maxval, file);
        map1_char = (unsigned char*)map1;

        if (e == STATIC){
            static_set_up_other_map(map1, map2, k_boundaries);
            map2_char = (unsigned char*)map2;
        } else if (e == ORDERED)
            init_to_zero(map1_char, k_boundaries);

    /*Make every process wait for process 0 to perform the task above*/
    MPI_Barrier(MPI_COMM_WORLD);

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

    if (e == ORDERED){
        printf("\nOrdered evolution to be implemented with MPI\n");
        // for(int i = 0; i < N_STEPS; i++){
        //     ordered_evolution(map1, k);
        //     #ifndef PROFILING
        //         sprintf(fname, "images/snapshots/snapshot%d.pgm", i);
        //         write_pgm_image(map1, maxval, k, k, fname);
        //     #endif
        // }
        exit(1);

    } else if (e == STATIC){
        for(int i = 0; i < N_STEPS; i++){

                /* Not using MPI_Scatter because I want to specify the rows to send exactly. */

            if (process_rank ==0){
                for (int i = 0; i < size_of_cluster; i++){
                    int start_row = i * (n_lines_per_process + 1);
                    int end_row = start_row + rows_to_send - 1;
                    if (end_row >= k_boundaries)
                        end_row = k_boundaries - 1; // Limit end_row to the matrix size

                    int rows_to_copy = end_row - start_row + 1;
                    MPI_Send(map1_char + start_row * k, rows_to_copy * k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
                }
            }

            int start_row = process_rank * (n_lines_per_process + 1);
            int end_row = start_row + rows_to_send - 1;
            if (end_row >= k_boundaries)
                end_row = k_boundaries - 1; // Limit end_row to the matrix size

            int rows_to_receive = end_row - start_row + 1;

            //Allocate space for local maps
            local_map1 = (unsigned char*)malloc(rows_to_receive * k * sizeof(unsigned char));
            local_map2 = (unsigned char*)malloc(rows_to_receive * k * sizeof(unsigned char));
            if (local_map1 == NULL || local_map2 == NULL){
                printf("Error: Could not allocate memory for local maps\n");
                exit(1);
            }

            MPI_Recv(local_map1, rows_to_receive * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            printf("Process %d received the submatrix\n", process_rank);

            static_evolution(local_map1, local_map2, k*n_lines_per_process, k);

            printf("Process %d finished the step\n", process_rank);

            // Gather the modified submatrices back to the root process
            // Note: This part needs adjustment based on how you want to gather the results
            // This simple example assumes gathering only the main portions without the extra rows
            if (process_rank == 0) {
                printf("Process %d: gathering the results\n", process_rank);     

                for (int i = 0; i < size_of_cluster; i++) {
                    int gather_start_row = i * n_lines_per_process;
                    int gather_rows_to_receive = n_lines_per_process;
                    MPI_Recv(map1_char + gather_start_row * k, gather_rows_to_receive * k, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            } else {
                int gather_start_row = (process_rank == 0) ? 0 : 1;
                int gather_rows_to_send = n_lines_per_process;
                MPI_Send(local_map1 + gather_start_row * k, gather_rows_to_send * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
            }

            if (process_rank == 0) {
                printf("Received full matrix\n");
            }
        }
    }

    #ifdef PROFILING
        double tend = CPU_TIME;
        double ex_time = tend-tstart;
        printf("\n\n Execution time is %f\n\n", ex_time);
    #endif
    
    free(map1);
    free(local_map1);
    free(local_map2);
    if (e == STATIC)
        free(map2);

    free(fname);
        
    MPI_Finalize();    
    return 0;
}
