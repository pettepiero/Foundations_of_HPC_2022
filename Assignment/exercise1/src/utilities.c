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
#include <dirent.h>
#include <unistd.h>

void command_line_parser(int *action, int *k, int *e, char **fname, int *n, int *s, int argc, char **argv){
    *action = 0;
    char *optstring = "irk:e:f:n:s:";

    int c;
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
    
        case 'i':
        	*action = INIT;
	break;
        
        case 'r':
        	*action = RUN;
	break;
        
        case 'k':
	        *k = atoi(optarg); 
	        if ((*k <= 0) || (*k >= 100000)){
		        printf("Error: matrix dimension k must be greater than 0 and smaller than 100000\n");
	        	exit(1);
	        }
		printf("Selected matrix dimension %d\n", *k);
        break;

        case 'e':
	        *e = atoi(optarg); 
	        if(*e == STATIC)
			printf("Using static evolution\n");
	        else if(*e == ORDERED)
	            	printf("Using ordered evolution\n");
	        else {
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
        break;

        case 'n':
        	*n = atoi(optarg);
        	if (*n <= 0)
        	{
        	    printf("Error: number of steps n must be greater than 0 \n");
        	    exit(1);
        	}
        break;

        case 's':
        	*s = atoi(optarg);
	break;

        default :
       		printf("argument -%c not known\n", c ); 
        	exit(1);
        break;
        }
    }

	if((*action != RUN) && (*action != INIT)){
		printf("No action specified\n");
	        printf("Possible actions:\n"
	               "r - Run a world\n"
	               "i - Initialize a world\n"
	               "Evolution types:\n"
	               "e 0 - Ordered evolution\n"
	               "e 1 - Static evolution\n\n");
	        exit(1);
	}
}


void set_up_map_variable(int action, int evolution, int k, void **map, int maxval, char *fname){
        /*  Determines if map has to be initialized or read from file.
            Then writes memory pointed by map variable after allocating it.
            Parameters:
                action: int, action to be performed, RUN or INIT
                evolution: int, evolution type, STATIC or ORDERED
		size: int, size of square matrix (side)
                map: unsigned char**, pointer to pointer to the map
                maxval: int, max value of the map
                fname: char*, pointer with name of the file
             */
	int nrows = k+ 2;
	int ncols = k;
	if (*map != NULL){
		printf("Error: *map is not NULL, risk of leakage\n");
		exit(1);
	}
        *map = (void *)calloc(nrows*ncols, sizeof(unsigned char));
	if (*map == NULL){
        	printf("Error: Could not allocate memory for map\n");
            	exit(1);
       	}

	if(action == RUN){
		if(fname == NULL){
			printf("File not specified. Running from images/blinker.pgm\n");
			printf("If you want to specify a different file, use -f\n"); 
        		fname = (char*)malloc( strlen("images/blinker.pgm")+1 );
        		if (fname == NULL)
        		{
        	    		printf("Error: Could not allocate memory for file name\n");
        	    		exit(1);
        		}
			strcpy(fname, "images/blinker.pgm");
		} else {
        		printf("Reading map from %s\n", fname);
		}
        	printf("******************************\nRunning a playground\n******************************\n");
		read_pgm_image(map, &maxval, &ncols, &nrows, fname);
		write_pgm_image(*map, maxval, ncols, nrows, fname);
	}
	else if(action == INIT){
		if(fname == NULL){
			printf("File not specified. Initializing as images/initial_image.pgm\n");
			printf("If you want to specify a different name, use -f\n"); 
        		fname = (char*)malloc( strlen("images/initial_image.pgm")+1 );
        		if (fname == NULL)
        		{
        	    		printf("Error: Could not allocate memory for file name\n");
        	    		exit(1);
        		}
			strcpy(fname, "images/initial_image.pgm");
		} else {
        		printf("Chosen name is %s\n", fname);
		}
        	printf("******************************\nInitializing a playground\n******************************\n");
		free(*map);
		*map = NULL;
        	*map = (unsigned char*)malloc(ncols*nrows*sizeof(unsigned char));
        	if (*map == NULL){
        	    printf("Error: Could not allocate memory for map\n");
        	    exit(1);
        	}
		
		#ifdef BLINKER
        	    #ifdef DEBUG
        	        printf("Generating blinker\n");
        	    #endif
        	    generate_blinker(*map, fname, ncols, nrows);
        	#endif

        	#ifndef BLINKER
		generate_map(*map, fname, 0.1, ncols, nrows, 0);
        	#endif
    	} else {
	        printf("Error, no action specified in set_up_map_variable\n");
	        exit(1);
	}
}

void static_set_up_other_map(unsigned char *map1, void **map2, int size){                                                                     /*  Allocates memory for map2 and copies map1 into it.
        Parameters:
            map1: unsigned char*, pointer to the first map
            map2: unsigned char**, pointer to pointer to the second map
        *   size: int, side of square matrix*/

        *map2 = (unsigned char*)malloc((size+2)*size*sizeof(unsigned char));
        if (map2 == NULL){
                printf("Error: Could not allocate memory for map2\n");
                exit(1); 
        }
	memcpy(*map2, map1, (size+2)*size*sizeof(unsigned char));
}                          

void print_map(int process_rank, int ncols, int rows_to_receive, unsigned char *map){

		printf("Process %d:\n", process_rank);
		for (int i=0; i<rows_to_receive; i++){
			for (int j=0; j<ncols; j++)
				printf("%4d", map[i*ncols+j]);
			printf("\n");
		}	
}

void calculate_rows_per_processor(int nrows, int nprocessors, int *rows_per_processor, int *start_indices){
	int ideal_nrows = nrows/nprocessors;
	int remainder = nrows%nprocessors;
	int start_row = 0;

	for (int i=0; i<nprocessors; i++){
		rows_per_processor[i] = ideal_nrows + (i < remainder? 1 : 0);
		start_indices[i] = start_row;

		if(i != 0){
			start_indices[i]--;
			rows_per_processor[i]++;
		}
		if(i == nprocessors-1){
			rows_per_processor[i] = nrows -1 - start_indices[i];
		}
		start_row = start_indices[i] + rows_per_processor[i]-1;
	}
}


void delete_pgm_files(const char *folder_path) {
    DIR *dir;
    struct dirent *entry;

    // Open the directory
    if ((dir = opendir(folder_path)) == NULL) {
        perror("opendir() error");
        return;
    }

    // Iterate over the directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Check if the file ends with .pgm
        if (strstr(entry->d_name, ".pgm") != NULL) {
            // Create the full path to the file
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", folder_path, entry->d_name);

            if (remove(filepath) != 0) {
                perror("remove() error");
            }
        }
    }

    closedir(dir);
}

