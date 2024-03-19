#ifndef READ_WRITE_PGM_IMAGE_H
#define READ_WRITE_PGM_IMAGE_H


void write_pgm_image( void *image, int maxval, int xsize, int ysize, const char *image_name);

void read_pgm_image( void **image, int *maxval, int *xsize, int *ysize, const char *image_name);

void swap_image( void *image, int xsize, int ysize, int maxval );

void * generate_gradient( int maxval, int xsize, int ysize );


#endif