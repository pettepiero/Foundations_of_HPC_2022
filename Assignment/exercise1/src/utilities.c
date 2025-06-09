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

void command_line_parser(Env *env, char **fname, int argc, char **argv)
{
	/* Setting default values first */
	env->action = 0;
	env->k = K_DFLT;
	env->e = STATIC;
	env->n = N_STEPS;
	env->s = 1;

	char *optstring = "irk:e:f:n:s:";

	int c;
	while ((c = getopt(argc, argv, optstring)) != -1)
	{
		switch (c)
		{

		case 'i':
			env->action = INIT;
			break;

		case 'r':
			env->action = RUN;
			break;

		case 'k':
			env->k = atoi(optarg);
			if ((env->k <= 0) || (env->k >= 100000))
			{
				printf("Error: matrix dimension k must be greater than 0 and smaller than 100000\n");
				exit(1);
			}
			env->nrows = env->k + 2;
			printf("Selected matrix dimension %d\n", env->k);
			printf("DEBUG: env->nrows = %d, env->k = %d\n", env->nrows, env->k);
			// Calculate the number of columns as multiples of the cache line (64 bytes unless changed in header file of constants)
			printf("DEBUG: going to use %d cols because of cache padding with cache line size %d\n", env->k + calculate_cache_padding(env->k), CACHE_LINE_SIZE);
			env->k += calculate_cache_padding(env->k);
			break;

		case 'e':
			env->e = atoi(optarg);
			if (env->e == STATIC)
				printf("Using static evolution\n");
			else if (env->e == ORDERED)
				printf("Using ordered evolution\n");
			else
			{
				printf("Unknown evolution type\n");
				exit(1);
			}
			break;

		case 'f':
			*fname = (char *)malloc(strlen(optarg) + 1);
			if (*fname == NULL)
			{
				printf("Error: Could not allocate memory for file name\n");
				exit(1);
			}
			strcpy(*fname, optarg);
			break;

		case 'n':
			env->n = atoi(optarg);
			if (env->n <= 0)
			{
				printf("Error: number of steps n must be greater than 0 \n");
				exit(1);
			}
			break;

		case 's':
			env->s = atoi(optarg);
			break;

		default:
			printf("argument -%c not known\n", c);
			exit(1);
			break;
		}
	}

	if ((env->action != RUN) && (env->action != INIT))
	{
		printf("No action specified\n");
		printf("Possible actions:\n"
			   "r - Run a world\n"
			   "i - Initialize a world\n"
			   "Evolution types:\n"
			   "e 0 - Ordered evolution\n"
			   "e 1 - Static evolution\n\n");
		printf("Choosing INIT by default\n");
		env->action = INIT;
	}
}

int calculate_next_processor(int process_num, int size_of_cluster)
{
	if (process_num >= size_of_cluster || process_num < 0)
	{
		printf("Error in 'calculate_next_processor': invalid process_num: %d\n", process_num);
		return size_of_cluster;
	}
	if (process_num != size_of_cluster - 1)
		return process_num + 1;
	else
		return 0;
}
int calculate_prev_processor(int process_num, int size_of_cluster)
{
	if (process_num >= size_of_cluster)
	{
		printf("Error in 'calculate_prev_processor': invalid process_num: %d\n", process_num);
		return size_of_cluster;
	}
	if (process_num != 0)
		return process_num - 1;
	else
		return size_of_cluster - 1;
}

void initialize_env_variable(Env *env)
{
	env->action = INIT;
	env->k = K_DFLT;
	env->e = STATIC;
	env->n = N_STEPS;
	env->s = 1;
	env->nrows = env->k + 2;
}

void convert_map_to_binary(unsigned char *map, int ncols, int nrows)
{
	/* Converts char map with values 0 or 255
	 * to values 0 or 1 using right shift */
	for (int i = 0; i < ncols * nrows; i++)
	{
		map[i] = map[i] >> 7;
	}
}

void convert_map_to_char(unsigned char *map, int ncols, int nrows)
{
	/* Converts char map with values 0 or 1
	 * to values 0 or 255 using right shift */
	for (int i = 0; i < ncols * nrows; i++)
	{
		map[i] = map[i] * 255;
	}
}

void set_up_map_variable(int action, int evolution, int k, void **map, int maxval, char *fname)
{
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
	int nrows = k + 2;
	int ncols = k;
	if (*map != NULL)
	{
		printf("Error: *map is not NULL, risk of leakage\n");
		exit(1);
	}
	*map = (void *)calloc(nrows * ncols, sizeof(unsigned char));
	if (*map == NULL)
	{
		printf("Error: Could not allocate memory for map in set_up_map_variable.\n");
		exit(1);
	}

	if (action == RUN)
	{
		if (fname == NULL)
		{
			printf("File not specified. Running from images/blinker.pgm\n");
			printf("If you want to specify a different file, use -f\n");
			fname = (char *)malloc(strlen("images/blinker.pgm") + 1);
			if (fname == NULL)
			{
				printf("Error: Could not allocate memory for file name\n");
				exit(1);
			}
			strcpy(fname, "images/blinker.pgm");
		}
		else
		{
			printf("Reading map from %s\n", fname);
		}
		printf("******************************\nRunning a playground\n******************************\n");
		read_pgm_image(map, &maxval, &ncols, &nrows, fname);
		write_pgm_image(*map, maxval, ncols, nrows, fname);
		convert_map_to_binary(*map, ncols, nrows);
	}
	else if (action == INIT)
	{
		if (fname == NULL)
		{
			printf("File not specified. Initializing as images/initial_image.pgm\n");
			printf("If you want to specify a different name, use -f\n");
			fname = (char *)malloc(strlen("images/initial_image.pgm") + 1);
			if (fname == NULL)
			{
				printf("Error: Could not allocate memory for file name\n");
				exit(1);
			}
			strcpy(fname, "images/initial_image.pgm");
		}
		else
		{
			printf("Chosen name is %s\n", fname);
		}
		printf("******************************\nInitializing a playground\n******************************\n");
		free(*map);
		*map = NULL;
		*map = (unsigned char *)malloc(ncols * nrows * sizeof(unsigned char));
		if (*map == NULL)
		{
			printf("Error: Could not allocate memory for map in initialization.\n");
			exit(1);
		}

#ifdef BLINKER
		generate_blinker(*map, fname, ncols, nrows);
#endif

#ifndef BLINKER
		generate_map(*map, fname, 0.1, ncols, nrows, 0);
#endif
	}
	else
	{
		printf("Error, no action specified in set_up_map_variable\n");
		exit(1);
	}
}

void static_set_up_other_map(unsigned char *map1, void **map2, int size)
{ /*  Allocates memory for map2 and copies map1 into it.
Parameters:
map1: unsigned char*, pointer to the first map
map2: unsigned char**, pointer to pointer to the second map
*   size: int, side of square matrix*/

	*map2 = (unsigned char *)malloc((size + 2) * size * sizeof(unsigned char));
	if (map2 == NULL)
	{
		printf("Error: Could not allocate memory for map2\n");
		exit(1);
	}
	memcpy(*map2, map1, (size + 2) * size * sizeof(unsigned char));
}

void print_map(int process_rank, int ncols, int rows_to_receive, unsigned char *map)
{

	printf("Process %d:\n", process_rank);
	for (int i = 0; i < rows_to_receive; i++)
	{
		for (int j = 0; j < ncols; j++)
			printf("%4d", map[i * ncols + j]);
		printf("\n");
	}
}

void print_env(Env env)
{
	printf("action = %d\n", env.action);
	printf("k = %d\n", env.k);
	printf("e = %d\n", env.e);
	printf("n= %d\n", env.n);
	printf("s = %d\n", env.s);
	printf("size_of_cluster = %d\n", env.size_of_cluster);
	printf("nrows = %d\n", env.nrows);
	printf("my_process_rows = %d\n", env.my_process_rows);
	printf("my_process_start_idx = %d\n", env.my_process_start_idx);
}

void calculate_rows_per_processor(Env env, int *rows_per_processor, int *start_indices)
{
	int tot_rows = env.nrows + 2 * (env.size_of_cluster - 1); // Counts overlaps between two clusters too
	int ideal_nrows = tot_rows / env.size_of_cluster;
	int remainder = tot_rows % env.size_of_cluster;
	int start_row = 0;

	for (int i = 0; i < env.size_of_cluster; i++)
	{
		rows_per_processor[i] = ideal_nrows + (i < remainder ? 1 : 0);
		start_indices[i] = start_row;

		if (i != 0)
		{
			start_indices[i]--; // overlapping extra row with previous processor
		}
		start_row = start_indices[i] + rows_per_processor[i] - 1;
	}
}

void delete_pgm_files(const char *folder_path)
{
	DIR *dir;
	struct dirent *entry;

	// Open the directory
	if ((dir = opendir(folder_path)) == NULL)
	{
		perror("opendir() error");
		return;
	}

	// Iterate over the directory entries
	while ((entry = readdir(dir)) != NULL)
	{
		// Check if the file ends with .pgm
		if (strstr(entry->d_name, ".pgm") != NULL)
		{
			// Create the full path to the file
			char filepath[1024];
			snprintf(filepath, sizeof(filepath), "%s/%s", folder_path, entry->d_name);

			if (remove(filepath) != 0)
			{
				perror("remove() error");
			}
		}
	}

	closedir(dir);
}

unsigned char *generate_blinker(unsigned char *restrict map, char fileName[], const int ncols, const int nrows)
{
	// Blinker
	map[((int)(nrows - 1) / 2 - 1) * ncols + (int)((ncols - 1) / 2)] = 255;
	map[((int)(nrows - 1) / 2) * ncols + (int)((ncols - 1) / 2)] = 255;
	map[((int)(nrows - 1) / 2 + 1) * ncols + (int)((ncols - 1) / 2)] = 255;

	write_pgm_image(map, MAXVAL, ncols, nrows, fileName);
	convert_map_to_binary(map, ncols, nrows);
	printf("PGM file created: %s with dimensions %d x %d\n", fileName, nrows, ncols);

	return map;
}

unsigned char *generate_map(unsigned char *restrict map, char *fname, const float probability, const int ncols, const int nrows, const int seed)
{
	if (seed == 0)
		srand(time(0));
	else
		srand(seed);

	// populate pixels
	for (int i = 1; i < nrows - 1; i++)
	{
		for (int j = 0; j < ncols; j++)
		{
			float random = (float)rand() / RAND_MAX;
			if (random < probability)
				map[i * ncols + j] = 255;
			else
				map[i * ncols + j] = 0;
		}
	}

	/* update boundary conditions (first and last rows)*/
	update_horizontal_edges(map, ncols, nrows);
	write_pgm_image(map, MAXVAL, ncols, nrows, fname);
	convert_map_to_binary(map, ncols, nrows);

	return map;
}

/* 	Returns the first multiple of CACHE_LINE_SIZE grater than the specified number of columns k
	This is done for efficiency purposes when working on cache lines and avoid false sharing*/
int calculate_cache_padding(int k)
{
	if (k <= 0)
	{
		printf("ERROR: k <= 0 in calculate_cache_line_padding\n");
		return 0;
	}
	if (k % CACHE_LINE_SIZE == 0)
		return 0;
	return CACHE_LINE_SIZE - k % CACHE_LINE_SIZE;
}
