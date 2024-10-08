#ifndef UTILITIES_H
#define UTILITIES_H
#include "constants.h"
#include "dynamics.h"

void 		command_line_parser(Env *env, char **fname, int argc, char **argv);
void 		initialize_env_variable(Env *env);
void 		set_up_map_variable(int action, int evolution, int size, void **map, int maxval, char file[]);
void 		static_set_up_other_map(unsigned char *map1, void **map2, int size);
void 		print_map(int process_rank, int k, int rows_to_receive, unsigned char *map);
void		print_env(Env env);
int 		nrows_given_process(int process_rank, int rows_per_process, int max_nrows);
void 		calculate_rows_per_processor(Env env, int *rows_per_processor, int* start_indices);
void 		delete_pgm_files(const char *folder_path);
void 		convert_map_to_binary(unsigned char * map, int ncols, int nrows);
void 		convert_map_to_char(unsigned char * map, int ncols, int nrows);
unsigned char 	*generate_blinker(unsigned char *restrict map, char fileName[], const int ncols, const int nrows); 
unsigned char 	*generate_map(unsigned char *restrict map, char* fname, const float probability, const int ncols, const int nrows, const int seed);

#endif //UTILITIES_H
