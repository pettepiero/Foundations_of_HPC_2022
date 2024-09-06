#ifndef UTILITIES_H
#define UTILITIES_H
#include "constants.h"
#include "dynamics.h"

void 	command_line_parser(int *action, int *k, int *e, char **fname, int *n, int *s, int argc, char **argv);
void 	set_up_map_variable(int action, int evolution, int size, void **map, int maxval, char file[]);
void 	static_set_up_other_map(unsigned char *map1, void **map2, int size);
void 	print_map(int process_rank, int rows_to_receive, int k, unsigned char *map);
int 	nrows_given_process(int process_rank, int rows_per_process, int max_nrows);
void 	calculate_rows_per_processor(int nrows, int nprocessors, int *rows_per_processor, int *start_indices);
#endif //UTILITIES_H
