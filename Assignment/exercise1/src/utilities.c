/* Piero Pettenà
Functions that make gol.c file cleaner and improve its readability */

#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"
#include "dynamics.h"
#include <omp.h>

void command_line_parser(int *action, int *k, int *e, char **fname, int *n, int *s, int argc, char **argv){
    *action = 0;
    char *optstring = "irk:e:f:n:s:";

    int c;
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
    
        case 'i':
        *action = INIT; break;
        
        case 'r':
        *action = RUN; break;
        
        case 'k':
        *k = atoi(optarg); 
        if ((*k <= 0) || (*k >= 100000)){
            printf("Error: matrix dimension k must be greater than 0 and smaller than 100000\n");
            exit(1);
        }
	printf("Selected matrix dimension %d\n.", *k);
        break;

        case 'e':
        *e = atoi(optarg); 
        if(*e == STATIC)
            printf("Using static evolution\n");
        else if(*e == ORDERED)
            printf("Using ordered evolution\n");
        else{
            printf("Unknown evolution type\n");
            exit(1);
        }
        break;

        case 'f':
        *fname = (char*)malloc( strlen(optarg)+1 );
        if (*fname == NULL)
        {
            printf("Error: Could not allocate memory for file name\n");
            exit(1);
        }
	strcpy(*fname, optarg);
        printf("Reading file: %s\n", *fname);
        break;

        case 'n':
        *n = atoi(optarg);
        if ((*n <= 0) || (*n >= 500))
        {
            printf("Error: number of steps n must be greater than 0 and less than 500\n");
            exit(1);
        }
        break;

        case 's':
        *s = atoi(optarg); break;

        default :
        printf("argument -%c not known\n", c ); 
        exit(1);
        break;
        }
    }
}


void set_up_map_variable(int action, int evolution, int size, void **map, int maxval, char file[]){
        /*  Determines if map has to be initialized or read from file.
            Then writes memory pointed by map variable after allocating it.
            Parameters:
                action: int, action to be performed, RUN or INIT
                evolution: int, evolution type, STATIC or ORDERED
		size: int, size of square matrix (side)
                map: unsigned char**, pointer to pointer to the map
                maxval: int, max value of the map
                file: char[], name of the file to read the map from
             */
    if(action == RUN){
        printf("******************************\nRunning a playground\n******************************\n");
	printf("NOTE: if k != K_DFLT, then new blinker of appropriate size will be created\n");
        printf("Reading map from %s\n", file);
	if(size != K_DFLT){
		free(*map);
		*map = NULL;
        	*map = (unsigned char*)malloc(size*size*sizeof(unsigned char));
	        if (*map == NULL){
            		printf("Error: Could not allocate memory for map\n");
            		exit(1);
       		}
		generate_blinker((void *)map, "images/blinker.pgm", size);
	}
	read_pgm_image((void *)map, &maxval, &size, &size, file);
		
	print_map(0, size, size, *map);

		
	write_pgm_image(*map, maxval, size, size, "images/copy_of_image.pgm");
        printf("Read map from %s\n", file);
    }
    else if(action == INIT){
	free(*map);
	*map = NULL;
        *map = (unsigned char*)malloc(size*size*sizeof(unsigned char));
        if (*map == NULL)
        {
            printf("Error: Could not allocate memory for map\n");
            exit(1);
        }
	
	#ifdef BLINKER
            #ifdef DEBUG
                printf("Generating blinker\n");
            #endif
            generate_blinker(map, "images/blinker.pgm", k);
        #endif

        #ifndef BLINKER
            generate_map(*map, "images/initial_map.pgm", 0.10, k, 0);
        #endif
        #ifdef DEBUG
            printf("Printing first 100 elements after create_map()\n");
            for(int i=0; i < 100; i++)
            {
                printf("%d ", ((unsigned char *)map)[i]);
            }
            printf("\n");
        #endif

    } else {
        printf("Error, no action specified in set_up_map_variable\n");
        exit(1);
    }
}

void static_set_up_other_map(unsigned char *map1, unsigned char *map2, int size){
    /*  Allocates memory for map2 and copies map1 into it.
        Parameters:
            map1: unsigned char*, pointer to the first map
            map2: unsigned char*, pointer to the second map
	*   size: int, side of square matrix
	*/
    map2 = (unsigned char*)malloc(size*size*sizeof(unsigned char));
    if (map2 == NULL)
    {
        printf("Error: Could not allocate memory for map2\n");
        exit(1);
    }
    memcpy(map2, map1, size*size*sizeof(unsigned char));
}

void print_map(int process_rank, int rows_to_receive, int k, unsigned char *map){

		printf("Process %d:\n", process_rank);
		for (int i=0; i<rows_to_receive+2; i++){
			for (int j=0; j<k; j++)
				printf("%d ", map[i*k+j]);
			printf("\n");
		}	
}

int nrows_given_process(int process_rank, int n_lines_per_process, int max_nrows){

	int start_row = process_rank * (n_lines_per_process);
	int end_row = start_row + n_lines_per_process- 1;

	if (end_row >= max_nrows)
		end_row = max_nrows- 1; // Limit end_row to the matrix size
	return end_row - start_row + 1;
}
