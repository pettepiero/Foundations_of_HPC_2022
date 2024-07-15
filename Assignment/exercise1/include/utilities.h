#ifndef UTILITIES_H
#define UTILITIES_H
#include "constants.h"
#include "dynamics.h"

void command_line_parser(int *action, int *k, int *e, char **fname, int *n, int *s, int argc, char **argv);
void set_up_map_variable(int action, int evolution, int size, void **map, int maxval, char file[]);
void static_set_up_other_map(unsigned char *map1, unsigned char *map2, int size);

#endif //UTILITIES_H
