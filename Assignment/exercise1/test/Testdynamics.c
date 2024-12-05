#include "unity.h"
#include "dynamics.h"
#include "utilities.h"
#include "constants.h"
#include "read_write_pgm_image.h"
#include <stdlib.h>
#include <unistd.h>

int nrows = K_DFLT +2;
int ncols = K_DFLT; 
int scols = 10;
int srows = 12;
int maxval = 255; 
unsigned char *map = NULL;
int expected_generated_map[] = {20, 80, 21, 42, 3, 13, 33, 43, 93, 103, 113, 44, 64, 15, 25, 65, 85, 95, 115, 36, 56, 96, 27, 67, 8, 18, 108, 118, 79, 89, 99};
char shift = sizeof(unsigned char)*8 -1;

//Comparison function
int compare(const void *a, const void *b){
	return (*(int *)a - *(int *)b);
}


void setUp()
{
    	map = (unsigned char*)calloc(nrows*ncols, sizeof(unsigned char));
	qsort(expected_generated_map, sizeof(expected_generated_map)/sizeof(int), sizeof(int), compare);
}

void tearDown()
{
	if (map) {
		free(map);
		map = NULL;
	}
	delete_pgm_files("./pgm/"); 
}


/****************************************************************/
/*		testing command_line_parser 			*/

void test_command_line_parser1(void) {
		optind = 1;
    		char *fname = NULL;
    		Env env;
		fname = NULL;
    		char *argv[] = { "program_name", "-i" };
    		int argc = 2;
    		command_line_parser(&env, &fname, argc, argv);
    		TEST_ASSERT_EQUAL_MESSAGE(1, env.action, "Test 1");
    		TEST_ASSERT_EQUAL_MESSAGE(K_DFLT, env.k, "Test 1");
    		TEST_ASSERT_EQUAL_MESSAGE(STATIC, env.e, "Test 1");
    		TEST_ASSERT_EQUAL_MESSAGE(N_STEPS, env.n, "Test 1");
    		TEST_ASSERT_EQUAL_MESSAGE(1, env.s, "Test 1");
    		TEST_ASSERT_NULL_MESSAGE(fname, "Test 1");  // Filename should not be allocated
}
void test_command_line_parser2(void){
		optind = 1;
    		char *fname = NULL;
    		Env env;
		fname = NULL;
    		char *argv2[] = { "program_name", "-r" };
    		int argc2 = 2;
    		command_line_parser(&env, &fname, argc2, argv2);
    		TEST_ASSERT_EQUAL_MESSAGE(RUN, env.action, "Test 2");
}
void test_command_line_parser3(void){
		optind = 1;
    		char *fname = NULL;
    		Env env;
		fname = NULL;
    		char *argv3[] = { "program_name", "-i", "-k", "50" };
    		int argc3 = 4;
    		command_line_parser(&env, &fname, argc3, argv3);
    		TEST_ASSERT_EQUAL_MESSAGE(50, env.k, "Test 3");
}
void test_command_line_parser4(void){
		optind = 1;
    		char *fname = NULL;
    		Env env;
		fname = NULL;
    		char *argv4[] = { "program_name", "-i", "-e", "1" };  // 1 is STATIC 
    		int argc4 = 4;
    		command_line_parser(&env, &fname, argc4, argv4);
    		TEST_ASSERT_EQUAL_MESSAGE(STATIC, env.e, "Test 4");
}
void test_command_line_parser5(void){
		optind = 1;
    		char *fname = NULL;
    		Env env;
		fname = NULL;
    		char *argv5[] = { "program_name", "-i", "-f", "data.txt" };
    		int argc5 = 4;
    		command_line_parser(&env, &fname, argc5, argv5);
    		TEST_ASSERT_NOT_NULL_MESSAGE(fname, "Test 5");
    		TEST_ASSERT_EQUAL_STRING_MESSAGE("data.txt", fname, "Test 5");
    		free(fname);
		fname = NULL;
}


/*	finished testing command_line_parser				*/
/************************************************************************/

void test_initialize_env_variable(){
	Env env;
	initialize_env_variable(&env);
	TEST_ASSERT_EQUAL_INT_MESSAGE(env.action, INIT, "Test on action");
	TEST_ASSERT_EQUAL_INT_MESSAGE(env.k, K_DFLT, "Test on k");
	TEST_ASSERT_EQUAL_INT_MESSAGE(env.e, STATIC, "Test on e");
	TEST_ASSERT_EQUAL_INT_MESSAGE(env.n, N_STEPS, "Test on n");
	TEST_ASSERT_EQUAL_INT_MESSAGE(env.s, 1, "Test on s");
	TEST_ASSERT_EQUAL_INT_MESSAGE(env.nrows, (env.k) +2, "Test on nrows");
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

void test_convert_map_to_binary()
{
    	free(map);
    	// Generating known map
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
//	printf("Generated map:\n");
//	print_map(0, scols, srows, map);
	for(int i=0;i <sizeof(expected_generated_map)/sizeof(int); i++){
		if(map[expected_generated_map[i]] != 1)
			printf("Invalid test_static_evolution: map[%d]=%d vs 1\n", expected_generated_map[i], map[expected_generated_map[i]]);
	}
	convert_map_to_char(map, scols, srows);
  	convert_map_to_binary(map, scols, srows); 

	unsigned char *correct_binary_map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));

	for(int i=0; i< scols*srows; i++){
		for (int j=0; j<sizeof(expected_generated_map)/sizeof(int); j++){
			if (i == expected_generated_map[j]){
				correct_binary_map[i] = 1;
				break;
			}
			else
				correct_binary_map[i] = 0;
		}
	}

/* Generated map
   0   0   0   1   0   0   0   0   1   0                                                                                                       
   0   0   0   1   0   1   0   0   1   0                                                                                                          
   1   1   0   0   0   1   0   1   0   0                                                                                                          
   0   0   0   1   0   0   1   0   0   0                                                                                                          
   0   0   1   1   1   0   0   0   0   0                                                                                                          
   0   0   0   0   0   0   1   0   0   0                                                                                                          
   0   0   0   0   1   1   0   1   0   0                                                                                                          
   0   0   0   0   0   0   0   0   0   1                                                                                                          
   1   0   0   0   0   1   0   0   0   1                                                                                                          
   0   0   0   1   0   1   1   0   0   1                                                                                                          
   0   0   0   1   0   0   0   0   1   0                                                                                                          
   0   0   0   1   0   1   0   0   1   0                                                                                                          
 */     
	TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(correct_binary_map, map, scols*srows, "correct_binary_map vs map\n");
}

void test_convert_map_to_char()
{
    	free(map);
    	// Generating known map
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
//	printf("Generated map\n");
//	print_map(0, scols, srows, map);
	for(int i=0;i <sizeof(expected_generated_map)/sizeof(int); i++){
		if(map[expected_generated_map[i]] != 1)
			printf("\n\nInvalid test_static_evolution: map[%d]=%d vs 1\n\n", expected_generated_map[i], map[expected_generated_map[i]]);
			break;
	}
  	convert_map_to_char(map, scols, srows); 
	//printf("Converted to char map\n");
	//print_map(0, scols, srows, map);
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

	unsigned char *correct_binary_map = (unsigned char*)calloc(scols*srows, sizeof(unsigned char));
	for(int i=0; i< scols*srows; i++){
		for (int j=0; j<sizeof(expected_generated_map)/sizeof(int); j++){
			if (i == expected_generated_map[j]){
				correct_binary_map[i] = 255;
				break;
			}
			else
				correct_binary_map[i] = 0;
		}
	}
	TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(correct_binary_map, map, scols*srows, "correct_binary_map vs map");
}

void test_count_alive_neighbours()
{
    	free(map);
    	// Generating known map
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
	//print_map(0, scols, srows, map);
  	shift_old_map(map, scols, srows, shift); 
/* Generated map
   0   0   0 128   0   0   0   0 128   0                                                                                                       
   0   0   0 128   0 128   0   0 128   0                                                                                                          
 128 128   0   0   0 128   0 128   0   0                                                                                                          
   0   0   0 128   0   0 128   0   0   0                                                                                                          
   0   0 128 128 128   0   0   0   0   0                                                                                                          
   0   0   0   0   0   0 128   0   0   0                                                                                                          
   0   0   0   0 128 128   0 128   0   0                                                                                                          
   0   0   0   0   0   0   0   0   0 128                                                                                                          
 128   0   0   0   0 128   0   0   0 128                                                                                                          
   0   0   0 128   0 128 128   0   0 128                                                                                                          
   0   0   0 128   0   0   0   0 128   0                                                                                                          
   0   0   0 128   0 128   0   0 128   0                                                                                                          
 */   
	 print_map_to_file(map, scols, srows, "./pgm/mapfile.txt");
   	 // Get number of alive neighbours for known cells
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(1, count_alive_neighbours(map, scols, 13), "test 1");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(3, count_alive_neighbours(map, scols, 22), "test 2");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(3, count_alive_neighbours(map, scols, 33), "test 3");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 65), "test 4");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 56), "test 5");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 108), "test 6");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(0, count_alive_neighbours(map, scols, 49), "test 7");
   	 TEST_ASSERT_EQUAL_INT_MESSAGE(2, count_alive_neighbours(map, scols, 10), "test 8");
}

void test_update_cell()
{
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(0), "This was test 1");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(1), "This was test 2");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(MAXVAL, update_cell(2), "This was test 3");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(MAXVAL, update_cell(3), "This was test 4");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(4), "This was test 5");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(0.5), "This was test 6");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(MAXVAL, update_cell(2.5), "This was test 7");
    TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, update_cell(-13), "This was test 8");
}


void test_shift_old_map()
{
    	free(map);
    	// Generating known map
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
//	printf("Generated map:\n");
//	print_map(0, scols, srows, map);
	/* Generated map
	   0   0   0   1   0   0   0   0   1   0
	   0   0   0   1   0   1   0   0   1   0
	   1   1   0   0   0   1   0   1   0   0
	   0   0   0   1   0   0   1   0   0   0
	   0   0   1   1   1   0   0   0   0   0
	   0   0   0   0   0   0   1   0   0   0
	   0   0   0   0   1   1   0   1   0   0
	   0   0   0   0   0   0   0   0   0   1
	   1   0   0   0   0   1   0   0   0   1
	   0   0   0   1   0   1   1   0   0   1
	   0   0   0   1   0   0   0   0   1   0
	   0   0   0   1   0   1   0   0   1   0
	* known solution is 128 (2^7) instead of 1 on each cell 
	*/
	shift_old_map(map, scols, srows, shift);
	unsigned char *known_solution= (unsigned char*)calloc(scols*srows, sizeof(unsigned char));
	for(int i=0; i< scols*srows; i++){
		for (int j=0; j<sizeof(expected_generated_map)/sizeof(int); j++){
			if (i == expected_generated_map[j]){
				known_solution[i] = 128;
				break;
			}
			else
				known_solution[i] = 0;
		}
	}

	TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(known_solution, map, scols*srows, "correct_binary_map vs map");
}


void test_is_alive()
{
	unsigned char a = 0;
	unsigned char b = 1;
	unsigned char c = 128;
	unsigned char d = 255;
	unsigned char e = 129;
	unsigned char f = 'a';
	unsigned char g = 'd';
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(&a), "Test 1");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(&b), "Test 2");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, is_alive(&c), "Test 3");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, is_alive(&d), "Test 4");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, is_alive(&e), "Test 5");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(&f), "Test 6");
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(&g), "Test 7");
}

//void test_static_evolution()
//{
//	if( (srows != 12) || (scols != 10))
//		printf("Error -> test_static_evolution can only be checked for srows=12, scols=101n");
//	
//    	//This test is done checking step from known configuration
//    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
//    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
//	//print_map(0, scols, srows, map);
// /* Generated map:                                                                                                                    
//   0   0   0   1   0   0   0   0   1   0
//   0   0   0   1   0   1   0   0   1   0
//   1   1   0   0   0   1   0   1   0   0
//   0   0   0   1   0   0   1   0   0   0
//   0   0   1   1   1   0   0   0   0   0
//   0   0   0   0   0   0   1   0   0   0
//   0   0   0   0   1   1   0   1   0   0
//   0   0   0   0   0   0   0   0   0   1
//   1   0   0   0   0   1   0   0   0   1
//   0   0   0   1   0   1   1   0   0   1
//   0   0   0   1   0   0   0   0   1   0
//   0   0   0   1   0   1   0   0   1   0
// */   
///* quick and not full test to verify if generated map is completely wrong */	
//	for(int i=0;i <sizeof(expected_generated_map)/sizeof(int); i++){
//	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(0), );
//	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(0), );
//	TEST_ASSERT_EQUAL_INT_MESSAGE(0, is_alive(0), );
//
//}

void test_static_evolution()
{
	if( (srows != 12) || (scols != 10))
		printf("Error -> test_static_evolution can only be checked for srows=12, scols=101n");
	
    	//This test is done checking step from known configuration
    	map = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	generate_map(map, "./pgm/seed10test.pgm", 0.2, scols, srows, 10);
	//print_map(0, scols, srows, map);
 /* Generated map:                                                                                                                    
   0   0   0   1   0   0   0   0   1   0
   0   0   0   1   0   1   0   0   1   0
   1   1   0   0   0   1   0   1   0   0
   0   0   0   1   0   0   1   0   0   0
   0   0   1   1   1   0   0   0   0   0
   0   0   0   0   0   0   1   0   0   0
   0   0   0   0   1   1   0   1   0   0
   0   0   0   0   0   0   0   0   0   1
   1   0   0   0   0   1   0   0   0   1
   0   0   0   1   0   1   1   0   0   1
   0   0   0   1   0   0   0   0   1   0
   0   0   0   1   0   1   0   0   1   0
 */   
/* quick and not full test to verify if generated map is completely wrong */	
	for(int i=0;i <sizeof(expected_generated_map)/sizeof(int); i++){
		if(map[expected_generated_map[i]] != 1)
			printf("Invalid test_static_evolution: map[%d]=%d vs 1\n", expected_generated_map[i], map[expected_generated_map[i]]);
	}
    	unsigned char* map2 = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    	unsigned char* map3 = (unsigned char*)calloc(srows*scols, sizeof(unsigned char));
    
    	static_evolution(map, scols, srows);
    	update_horizontal_edges(map, scols, srows);
	/* Set all old MSB to 0 */
	for(int i=0; i< scols*srows; i++){
		map[i] = map[i] & 0x7F;
	}	


 /* Known solution:                                                                                                                    
 * 
   0   0   1   1   0   1   1   1   1   1
   1   1   1   0   0   0   1   1   1   1
   0   0   1   1   0   1   0   1   1   1  
   1   1   0   1   0   1   1   1   0   0
   0   0   1   1   1   1   1   1   0   0
   0   0   1   0   0   0   1   1   0   0
   0   0   0   0   0   1   1   0   1   0 
   1   0   0   0   1   1   1   0   1   1
   1   0   0   0   1   1   1   0   1   1
   1   0   1   0   0   1   1   1   1   1
   0   0   1   1   0   1   1   1   1   1
   1   1   1   0   0   0   1   1   1   1
 */   

	int active_bits[] = {2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 16, 17, 18, 19, 22, 23, 25, 27, 28, 29, 30, 31, 33, 35, 36, 37, 42, 43, 44, 45, 46, 47, 52, 56, 57, 65, 66, 68, 70, 74, 75, 76, 78, 79, 80, 84, 85, 86, 88, 89, 90, 92, 95, 96, 97, 98, 99, 102, 103, 105, 106, 107, 108, 109, 110, 111, 112, 116, 117, 118, 119}; 

    	for(int i = 0; i <sizeof(active_bits)/sizeof(int); i++)
    	{
    	    map3[active_bits[i]] = 1;
    	}
    	//Check each element
    	for(int i = 0; i < srows*scols; i++){
    	    	TEST_ASSERT_EQUAL_CHAR_MESSAGE(map3[i], map[i], "This was test 1");
    	}
}


void test_calculate_rows_per_processor(){
	Env env;
	int rows_per_processor[env.size_of_cluster];
	int start_indices[env.size_of_cluster];
	env.size_of_cluster = 4;
	env.nrows = 23;
	env.k = 21;
	int known_result[] = {8, 7, 7, 7};
	calculate_rows_per_processor( env, rows_per_processor, start_indices);
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(known_result, rows_per_processor, 4, "Test 1");
	
	env.size_of_cluster = 3;
	env.nrows = 52;
	env.k = 50;
	int known_result2[] = {19, 19, 18};
	calculate_rows_per_processor(env, rows_per_processor, start_indices);
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(known_result2, rows_per_processor, 3, "Test 2");
}

int main(void)
{
    	UNITY_BEGIN();

    	RUN_TEST(test_update_horizontal_edges);
    	RUN_TEST(test_count_alive_neighbours);
	RUN_TEST(test_is_alive);
    	RUN_TEST(test_update_cell);
	RUN_TEST(test_shift_old_map);
    	RUN_TEST(test_static_evolution);
    	RUN_TEST(test_convert_map_to_binary);
    	RUN_TEST(test_convert_map_to_char);
	RUN_TEST(test_command_line_parser1);
	RUN_TEST(test_command_line_parser2);
	RUN_TEST(test_command_line_parser3);
	RUN_TEST(test_command_line_parser4);
	RUN_TEST(test_command_line_parser5);
	RUN_TEST(test_initialize_env_variable);
	RUN_TEST(test_calculate_rows_per_processor);
    	UNITY_END();

    	return 0;
}
