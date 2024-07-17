/*

This is custom mutator for AFL++ to fuzz *.gif files.

Docs: https://github.com/AFLplusplus/AFLplusplus/blob/stable/docs/custom_mutators.md

*/

#include "/home/anton/AFLplusplus/include/afl-fuzz.h" // You need to use -I/path/to/AFLplusplus/include -I.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gif_generator.h"

#define DATA_SIZE (2000)

//My structure with info about mutation
typedef struct my_mutator {

  afl_state_t *afl;

  gif_object_t gif;

  size_t trim_size_current;
  int    trimmming_steps;
  int    cur_step;

  uint8_t *mutated_out, *post_process_buf, *trim_buf;

} my_mutator_t;

#define CHECK_PTR(ptr, err_msg) \
	if (ptr == NULL) 			\
	{							\
		perror(err_msg);		\
		return NULL;			\
	}

//=======================================================================================

/*
	Initialize data custom mutation
*/

my_mutator_t *afl_custom_init(afl_state_t *afl, unsigned int seed) 
{
	srand(seed);  // needed also by surgical_havoc_mutate()

	my_mutator_t *data = (my_mutator_t *) calloc(1, sizeof(my_mutator_t)); //init my structure
    CHECK_PTR(data, "afl_custom_init calloc data");

	data->mutated_out      = (uint8_t *)malloc(MAX_FILE);
	CHECK_PTR(data->mutated_out, "afl_custom_init malloc mutated_out")

	data->post_process_buf = (uint8_t *)malloc(MAX_FILE);
	CHECK_PTR(data->post_process_buf, "afl_custom_init malloc post_process_buf")

	data->trim_buf         = (uint8_t *)malloc(MAX_FILE);
	CHECK_PTR(data->trim_buf, "afl_custom_init malloc trim_buf")

	data->afl = afl;

  	return data;
}

/*
	Perform custom mutations on a given input
*/

size_t afl_custom_fuzz(	my_mutator_t 	*data, 			//data pointer returned in afl_custom_init for this fuzz case
						uint8_t 		*buf, 			//buf Pointer to input data to be mutated
						size_t 			buf_size,		//buf_size Size of input data
                       	uint8_t 		**out_buf, 		//out_buf the buffer we will work on. we can reuse *buf. NULL on error.
						uint8_t 		*add_buf,		//add_buf Buffer containing the additional test case
                       	size_t 			add_buf_size,  	//add_buf_size Size of the additional test case. add_buf can be NULL
                       	size_t 			max_size		//max_size Maximum size of the mutated output. The mutation must not
														//					produce data larger than max_size.
						//return Size of the mutated output.
					) 
{
	// Make sure that the packet size does not exceed the maximum size expected by the fuzzer
  	size_t mutated_size = DATA_SIZE <= max_size ? DATA_SIZE : max_size;
	//Starts mutation
	uint8_t gen_gif_buffer[DATA_SIZE] = {};
	data->gif.data = gen_gif_buffer;
	data->gif.pos = 0;
	generate_gif(&(data->gif));

	if (max_size < data->gif.pos) {
        memcpy(data->mutated_out, data->gif.data, max_size);
        mutated_size = max_size;
    } else {
        memcpy(data->mutated_out, data->gif.data, data->gif.pos);
        mutated_size = data->gif.pos;
    }
    
	//Ends mutation
	if (mutated_size > max_size) { mutated_size = max_size; } //mutated_size - new size of mutated output

  	*out_buf = data->mutated_out;	//set out_buf to new mutated buffer
  	return mutated_size;
}

/*
	Deinitialize everything
*/

void afl_custom_deinit(my_mutator_t *data) {
  free(data->post_process_buf);
  free(data->mutated_out);
  free(data->trim_buf);
  free(data);
}