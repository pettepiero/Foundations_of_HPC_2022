#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "read_write_pgm_image.h"
#include "constants.h"

#ifdef BLINKER
unsigned char *generate_blinker(unsigned char *restrict map, char fileName[], int size){
    // Blinker
    map[(int)(SIDE)/2 * SIDE + (int)(SIDE /2)] = 255;
    map[((int)(SIDE)/2 +1) * SIDE + (int)(SIDE /2)] = 255;
    map[((int)(SIDE)/2 +2)* SIDE + (int)(SIDE /2)] = 255;

    write_pgm_image(map, maxval, size, size, fileName);
    printf("PGM file created: %s\n", fileName);
    return map;
}
#endif


void update_edges(unsigned char *restrict map, const int size)
{
    // update top and bottom edges
    for (int i=0; i<size; i++)
    {
        map[i] = map[i+size*(size-1)];
        map[i+size*(size-1)] = map[i+size];
    }

    // update left and right edges
    for (int i=0; i< size; i++)
    {
        map[i*size] = map[i*size + size-2];
        map[i*size + size-1] = map[i*size + 1];
    }
}

unsigned char *generate_map(unsigned char *restrict map, const char fileName[], const float probability, const int size){
    
    srand(time(0));
    float rand_max_inverse = 1 / RAND_MAX;
    // Populate pixels
    for (int i = 0; i < SIDE; i++) {
        for (int j = 0; j < SIDE; j++) {

        }
    }

    for (int i=0; i<SIDE*SIDE; i++){
        if ((float)rand() *rand_max_inverse < probability)
            map[i] = 255;
        else
            map[i] = 0;
    }
    update_edges(map, size);

    write_pgm_image(map, maxval, size, size, fileName);
    printf("PGM file created: %s\n", fileName);
    return map;
}

// A cell that did not change at the last time step, and none of whose neighbors changed,
// is guaranteed not to change at the current time step as well, so a program that keeps 
// track of which areas are active can save time by not updating inactive zones.
void get_active_zones()
{}


// Defenitely the wrong way to do it but it's a start
int count_alive_neighbours(unsigned char *restrict map, const int size, const int index)
{
    int count = 0;
    int neighbours[] = {index-1, index+1, index-size, index+size, index-size-1, index-size+1, index+size-1, index+size+1};

    #ifdef DEBUG
    printf("Neighbours of %d: \n", index);
    for (int i=0; i<8; i++)
    {
        printf("%d=%d \n", neighbours[i], map[neighbours[i]]);
    }
    #endif

    for (int i = 0; i < 8; i++)
        if (map[neighbours[i]] == 255)
            count++;


    return count;
}

// I believe the compiler will optimize this with a ternary operation.
// I have left it so for the sake of readability
char update_cell(const int alive_neighbours)
{
    if(alive_neighbours < 2 || alive_neighbours > 3)
        return 0;
    else
        return 255;
}

// Performs a single step of the update of the map
void update_map(unsigned char *restrict current, unsigned char *restrict new, int size)
{
    #ifdef DEBUG
    printf("Inside update_map.\n");
    #endif
    int i = 0;
    for(int row=1; row < size-1; row++)
        for(int col=1; col < size-1; col++)    
    {
        i = row*size + col;
        // check neighbours
        int alive_counter = count_alive_neighbours(current, size, i);
        // update element
        new[i] = update_cell(alive_counter);
        #ifdef DEBUG
        printf("row = %d, col %d, i = %d\n", row, col, i);  
        printf("Cell %d, counted %d alive cells, new[%d] = %d\n", i, alive_counter, i , new[i]);
        #endif
    }

    update_edges(new, size);

    memcpy(current, new, size*size*sizeof(char));

    #ifdef DEBUG
    printf("Updated map\n");
    printf("Printing first 100 elements\n");
    for(int i=0; i < 100; i++)
    {
        printf("%d ", current[i]);
    }
    #endif
}