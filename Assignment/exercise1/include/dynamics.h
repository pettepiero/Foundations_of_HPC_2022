#ifndef DYNAMICS_H
#define DYNAMICS_H
#include "constants.h"

#ifdef BLINKER
unsigned char *generate_blinker(unsigned char* map, char fileName[], int size);
#endif

unsigned char *generate_map(unsigned char* map, char fileName[], float probability, int size);
void get_active_zones();
void get_wrapped_neighbors(const int num_rows, const int num_cols, int index, int neighbors[]);
int count_alive_neighbours(unsigned char *map, int size, int index);
char update_cell(int alive_neighbours);
void update_map(unsigned char *current, unsigned char *new, int size);





#endif // DYNAMICS_H