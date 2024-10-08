#ifndef DYNAMICS_H
#define DYNAMICS_H
#include "constants.h"
#include "utilities.h"

unsigned char 	*generate_blinker(unsigned char *restrict map, char fileName[], const int ncols, const int nrows);

unsigned char   *generate_map(unsigned char* map, char fileName[], float probability, const int ncols, const int nrows, const int seed);
void            get_active_zones();
void            get_wrapped_neighbors(const int num_rows, const int num_cols, int index, int neighbors[]);
int             count_alive_neighbours(unsigned char *restrict map, int size, int index);
char            update_cell(int alive_neighbours);
void 		ordered_evolution(unsigned char *restrict map, int ncols, int nrows);
void 		static_evolution(unsigned char *restrict current, int ncols, int n_inner_rows);
void		update_horizontal_edges(unsigned char *restrict map, const int ncols, const int nrows);
void            init_to_zero(unsigned char *restrict map1, const int k);
void 		print_map_to_file(unsigned char *restrict map, const int ncols, const int nrows, const char fileName[]);
void 		shift_old_map(unsigned char *restrict map, const int ncols, const int nrows, const char shift);
int 		is_alive(unsigned char *value);
void 		split_initial_matrix(unsigned char *restrict map1, const Env env, const int start_indices[]);

#endif // DYNAMICS_H
