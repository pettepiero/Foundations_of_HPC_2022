#include "unity.h"
#include "dynamics.h"
#include "utilities.h"
#include "constants.h"
#include "read_write_pgm_image.h"
#include <stdlib.h>

int nrows = K_DFLT +2;
int ncols = K_DFLT; 
int scols = 10;
int srows = 12;
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
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
	print_map(0, scols, srows, map);
   
/* Generated map
   0   0   0 255   0   0   0   0 255   0                                                                                                       
   0   0   0 255   0 255   0   0 255   0                                                                                                          
 255 255   0   0   0 255   0 255   0   0                                                                                                          
   0   0   0 255   0   0 255   0   0   0                                                                                                          
   0   0 255 255 255   0   0   0   0   0                                                                                                          
   0   0   0   0   0   0 255   0   0   0                                                                                                          
   0   0   0   0 255 255   0 255   0   0                                                                                                          
   0   0   0   0   0   0   0   0   0 255                                                                                                          
 255   0   0   0   0 255   0   0   0 255                                                                                                          
   0   0   0 255   0 255 255   0   0 255                                                                                                          
   0   0   0 255   0   0   0   0 255   0                                                                                                          
   0   0   0 255   0 255   0   0 255   0                                                                                                          
 */   
	print_map_to_file(map, scols, srows, "./pgm/mapfile.txt");
   	 // Get number of alive neighbours for known cells
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(1, count_alive_neighbours(map, scols, 13), "test 1\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(3, count_alive_neighbours(map, scols, 22), "test 2\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(3, count_alive_neighbours(map, scols, 33), "test 3\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 65), "test 4\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 56), "test 5\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 108), "test 6\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(0, count_alive_neighbours(map, scols, 49), "test 7\n");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 10), "test 8\n");
	
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
	if( (srows != 12) || (scols != 10))
		printf("Error -> test_static_evolution can only be checked for srows=12, scols=101n");
	
    	//This test is done checking step from known configuration
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
	printf("Inside test_static_evolution():\n");
	print_map(0, scols, srows, map);
 /* Generated map:                                                                                                                    
   0   0   0 255   0   0   0   0 255   0
   0   0   0 255   0 255   0   0 255   0
 255 255   0   0   0 255   0 255   0   0
   0   0   0 255   0   0 255   0   0   0
   0   0 255 255 255   0   0   0   0   0
   0   0   0   0   0   0 255   0   0   0
   0   0   0   0 255 255   0 255   0   0
   0   0   0   0   0   0   0   0   0 255
 255   0   0   0   0 255   0   0   0 255
   0   0   0 255   0 255 255   0   0 255
   0   0   0 255   0   0   0   0 255   0
   0   0   0 255   0 255   0   0 255   0
 */   
	int expected_generated_map[] = {20, 80, 21, 42, 3, 13, 33, 43, 93, 103, 113, 44, 64, 15, 25, 65, 85, 95, 115, 36, 56, 96, 27, 67, 8, 18, 108, 118, 79, 89, 99};
/* quick and not full test to verify if generated map is completely wrong */	
	for(int i=0;i <sizeof(expected_generated_map)/sizeof(int); i++){
		if(map[expected_generated_map[i]] != 255)
			printf("Invalid test_static_evolution: map[%d]=%d vs 255\n", expected_generated_map[i], map[expected_generated_map[i]]);
	}

 
    unsigned char* map2 = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    unsigned char* map3 = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    
    static_evolution(map, map2, scols, srows);
 /* Known solution:                                                                                                                    
 * 
   0   0 255 255   0 255 255 255 255 255
 255 255 255   0   0   0 255 255 255 255
   0   0 255 255   0 255   0 255 255 255  
 255 255   0 255   0 255 255 255   0   0
   0   0 255 255 255 255 255 255   0   0
   0   0 255   0   0   0 255 255   0   0
   0   0   0   0   0 255 255   0 255   0 
 255   0   0   0 255 255 255   0 255 255
 255   0   0   0 255 255 255   0 255 255
 255   0 255   0   0 255 255 255 255 255
   0   0 255 255   0 255 255 255 255 255
 255 255 255   0   0   0 255 255 255 255
 */   

	int active_bits[] = {2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 16, 17, 18, 19, 22, 23, 25, 27, 28, 29, 30, 31, 33, 35, 36, 37, 42, 43, 44, 45, 46, 47, 52, 56, 57, 65, 66, 68, 70, 74, 75, 76, 78, 79, 80, 84, 85, 86, 88, 89, 90, 92, 95, 96, 97, 98, 99, 102, 103, 105, 106, 107, 108, 109, 110, 111, 112, 116, 117, 118, 119}; 

    	for(int i = 0; i <sizeof(active_bits)/sizeof(int); i++)
    	{
    	    map3[active_bits[i]] = 255;
    	}
    	update_horizontal_edges(map2, scols, srows);
    	//Check each element
    	for(int i = 0; i < srows*scols; i++){
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
