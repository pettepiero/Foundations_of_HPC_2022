#ifndef DYNAMICS_H
#define DYNAMICS_H
#include "constants.h"
#include "utilities.h"

unsigned char 	*generate_blinker(unsigned char *restrict map, char fileName[], const int ncols, const int nrows);
unsigned char   *generate_map(unsigned char* map, char fileName[], float probability, const int ncols, const int nrows, const int seed);
void            get_wrapped_neighbors(const int num_rows, const int num_cols, int index, int neighbors[]);
int             count_alive_neighbours_multi(unsigned char *restrict map, int size, int index);
int             count_alive_neighbours_single(unsigned char *restrict map, int size, int index);
char            update_cell(int alive_neighbours);
void 		    ordered_evolution(unsigned char *restrict map, int ncols, int nrows);
void            static_evolution(unsigned char *restrict current, int ncols, int nrows, char shift);
void		    update_horizontal_edges(unsigned char *restrict map, const int ncols, const int nrows);
void                init_to_zero(unsigned char *restrict map1, const int k);
void 		    print_map_to_file(unsigned char *restrict map, const int ncols, const int nrows, const char fileName[]);
void 		    shift_old_map(unsigned char *restrict map, const int ncols, const int nrows, const char shift);
void 		    mask_MSB(unsigned char *restrict map, const int ncols, const int nrows);
int 		    is_alive(unsigned char *value);
// inline int is_alive(unsigned char *value){
// 	return (*value & 0x80) >> (sizeof(unsigned char)*8 -1);
// }
void 		    split_initial_matrix(unsigned char *restrict map1, const Env env, const int start_indices[]);
void 		    gather_submaps(unsigned char* restrict map, const Env env, const int *start_indices, const int *rows_per_processor);
void 		    send_submaps(unsigned char* restrict map, const Env env, const int *rows_per_processor, const int process_rank);

#endif // DYNAMICS_H
