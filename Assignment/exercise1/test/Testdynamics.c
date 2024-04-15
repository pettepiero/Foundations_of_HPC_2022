#include "unity.h"
#include "dynamics.h"
#include "constants.h"
#include "read_write_pgm_image.h"
#include <stdlib.h>

int k = K_DFLT +2;
int maxval = 255;
unsigned char *map = NULL;

void setUp()
{
    map = (unsigned char*)calloc(k*k, sizeof(unsigned char));
}

void tearDown()
{
    free(map);
    //delete pgm files
}

void test_update_edges()
{
    
    generate_map(map, "./pgm/test.pgm", 0.5, k, 0);

    // update edges
    update_edges(map, k);
    
    // Get outer edges
    int left[k-2], right[k-2], top[k-2], bottom[k-2];
    for (int i=1; i<k-1; i++){
        left[i-1] = map[i*k];
        right[i-1] = map[i*k + k-1];
        top[i-1] = map[i];
        bottom[i-1] = map[i + k*(k-1)];
    }
    // Get inner edges
    int left_inner[k-2], right_inner[k-2], top_inner[k-2], bottom_inner[k-2];
    for (int i=1; i<k-1; i++){
        left_inner[i-1] = map[i*k + 1];
        right_inner[i-1] = map[i*k + k-2];
        top_inner[i-1] = map[i+k];
        bottom_inner[i-1] = map[i+k*(k-2)];
    }
    // Get angles
    int top_left = map[0], top_right = map[k-1], bottom_left = map[k*(k-1)], bottom_right = map[k*k-1];
    // Get inner angles
    int top_left_inner = map[k+1], top_right_inner = map[2*k-2],
    bottom_left_inner = map[k*(k-2)+1], bottom_right_inner = map[k*(k-2)+k-2];
    
    // Perform test
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(left, right_inner, k-2, "This was test 1\n");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(right, left_inner, k-2, "This was test 2\n");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(top, bottom_inner, k-2, "This was test 3\n");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(bottom, top_inner, k-2, "This was test 4\n");

    TEST_ASSERT_EQUAL_INT_MESSAGE(top_left, bottom_right_inner, "This was test 5\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(top_right, bottom_left_inner, "This was test 6\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(bottom_left, top_right_inner, "This was test 7\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(bottom_right, top_left_inner, "This was test 8\n");
}

void test_count_alive_neighbours()
{
    free(map);
    // Generating known map
    map = (unsigned char*)calloc(10*10, sizeof(unsigned char));
    generate_map(map, "./pgm/seed10test.pgm", 0.5, 10, 10);
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

    //print_map_to_file(map, 10, "./pgm/mapfile.txt");

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

void test_update_map()
{
    //This test is done checking step from known configuration
    map = (unsigned char*)calloc(10*10, sizeof(unsigned char));
    generate_map(map, "./pgm/seed10test.pgm", 0.5, 10, 10);
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

    update_map(map, map2, 10);
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

    RUN_TEST(test_update_edges);
    RUN_TEST(test_update_edges);
    RUN_TEST(test_count_alive_neighbours);
    RUN_TEST(test_update_cell);
    RUN_TEST(test_update_map);

    UNITY_END();

    return 0;
}