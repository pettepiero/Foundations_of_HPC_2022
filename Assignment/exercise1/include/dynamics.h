#ifndef DYNAMICS_H
#define DYNAMICS_H
#include "constants.h"

#ifdef BLINKER
unsigned char *generate_blinker(unsigned char* map, char fileName[], int size);
#endif

unsigned char *generate_map(unsigned char* map, char fileName[], float probability, int size, const int seed);
void get_active_zones();
void get_wrapped_neighbors(const int num_rows, const int num_cols, int index, int neighbors[]);
int count_alive_neighbours(unsigned char *map, int size, int index);
char update_cell(int alive_neighbours);
void ordered_evolution(unsigned char *restrict map, int size);
void static_evolution(unsigned char *current, unsigned char *new, int size);
void update_edges(unsigned char *restrict map, const int size);
void print_map(unsigned char *restrict map, const int k);
void init_to_zero(unsigned char *restrict map1, const int k);





#endif // DYNAMICS_H