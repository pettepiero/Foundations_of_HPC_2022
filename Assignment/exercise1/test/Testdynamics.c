#include "unity.h"
#include "dynamics.h"
#include "constants.h"
#include "read_write_pgm_image.h"
#include <stdlib.h>

int nrows = K_DFLT +2;
int ncols = K_DFLT; 
int maxval = 255; 
unsigned char *map = NULL;

void setUp()
{
    map = (unsigned char*)calloc(nrows*ncols, sizeof(unsigned char));
}

void tearDown()
{
    free(map);
    //delete pgm files
}

void test_update_horizontal_edges()
{
	generate_map(map, "./pgm/test.pgm", 0.5, ncols, nrows, 0);
	
	// update edges
	update_horizontal_edges(map, ncols, nrows);
	    
	// Get outer edges
	int top[nrows], bottom[nrows];
	for (int i=0; i<ncols; i++){
        	top[i] = map[i];
        	bottom[i] = map[i + ncols*(nrows-1)];
	}

   	 // Get inner edges
   	 int top_inner[ncols], bottom_inner[ncols];
   	 for (int i=0; i<ncols; i++){	
   	     	top_inner[i] = map[i+ncols];
   	     	bottom_inner[i] = map[i+(nrows-2)*ncols];
   	 }
   	 
   	 // Perform test
   	 TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(top, bottom_inner, ncols, "Top vs bottom inner\n");
   	 TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(bottom, top_inner, ncols, "Bottom vs top inner\n");
}

void test_count_alive_neighbours()
{
    free(map);
    // Generating known map
    map = (unsigned char*)calloc(10*10, sizeof(unsigned char));
    generate_map(map, "./pgm/seed10test.pgm", 0.5, 10, 10, 10);
    /* Generated map:
    255     0       0       255     255     255     255     0       255     0
    0       255     0       0       0       255     255     255     0       255
    255     0       0       255     255     255     255     0       255     0
    255     0       255     255     255     0       255     0       255     0
    255     255     255     0       255     255     255     0       255     255
    0       0       0       255     255     255     255     255     0       0
    255     0       0       0       0       0       0       0       255     0
    0       255     0       255     255     255     0       255     0       255
    255     0       0       255     255     255     255     0       255     0
    0       255     0       0       0       255     255     255     0       255
    */

    print_map_to_file(map, 10, 10, "./pgm/mapfile.txt");

    // Get number of alive neighbours for known cells
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, 10, 11), "This was test 1\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, count_alive_neighbours(map, 10, 22), "This was test 2\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(6, count_alive_neighbours(map, 10, 33), "This was test 3\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(7, count_alive_neighbours(map, 10, 15), "This was test 4\n");
}

void test_update_cell()
{
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(0), "This was test 1 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(1), "This was test 2 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(255, update_cell(2), "This was test 3 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(255, update_cell(3), "This was test 4 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(4), "This was test 5 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(0.5), "This was test 6 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(255, update_cell(2.5), "This was test 7 \n");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(-13), "This was test 8 \n");
}
//static_evolution(unsigned char *restrict current, unsigned char *restrict new, int ncols, int n_inner_rows);

void test_static_evolution()
{
    //This test is done checking step from known configuration
    map = (unsigned char*)calloc(10*10, sizeof(unsigned char));
    generate_map(map, "./pgm/seed10test.pgm", 0.5, 10, 10, 10);
    /* Generated map:
    255     0       0       255     255     255     255     0       255     0
    0       255     0       0       0       255     255     255     0       255
    255     0       0       255     255     255     255     0       255     0
    255     0       255     255     255     0       255     0       255     0
    255     255     255     0       255     255     255     0       255     255
    0       0       0       255     255     255     255     255     0       0
    255     0       0       0       0       0       0       0       255     0
    0       255     0       255     255     255     0       255     0       255
    255     0       0       255     255     255     255     0       255     0
    0       255     0       0       0       255     255     255     0       255
    */
    unsigned char* map2 = (unsigned char*)calloc(10*10, sizeof(unsigned char));
    unsigned char* map3 = (unsigned char*)calloc(10*10, sizeof(unsigned char));
    
    static_evolution(map, map2, 10, 8);
    //Known solution
    /*
    0       255     0       255     0       0       0       0       0       255
    0       255     255     0       0       0       0       0       0       255    
    255     0       0       0       0       0       0       0       255     0
    255     0       0       0       0       0       0       0       255     0
    255     0       0       0       0       0       0       0       255     0
    0       0       255     255     0       0       0       0       0       0
    255     255     255     0       0       0       0       0       255     255
    0       255     255     255     0       0       0       255     0       255
    0       255     0       255     0       0       0       0       0       255
    0       255     255     0       0       0       0       0       0       255
    */
    int active_bits[]= {1, 3, 9, 11, 12, 19, 20, 28, 30, 38, 40, 48,
                        52, 53, 
                        60, 61, 62, 68, 69, 71, 72, 73, 77, 79,
                        81, 83, 89, 91, 92, 99};
    for(int i = 0; i <sizeof(active_bits)/sizeof(int); i++)
    {
        map3[active_bits[i]] = 255;
    }

    //Check each element
    for(int i = 0; i < 10*10; i++)
    {
        TEST_ASSERT_EQUAL_CHAR_MESSAGE(map3[i], map2[i], "This was test 1\n");
    }
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_update_horizontal_edges);
    RUN_TEST(test_count_alive_neighbours);
    RUN_TEST(test_update_cell);
    RUN_TEST(test_static_evolution);

    UNITY_END();

    return 0;
}
