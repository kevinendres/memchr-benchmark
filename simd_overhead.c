//Returns time of memchr operation in nanoseconds
//tests for overhead in SIMD instructions

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 100000
#endif

#ifndef MEM_FILLER
# define MEM_FILLER 0x42
#endif

#ifndef SEARCH_STR
# define SEARCH_STR 0x41
#endif

#ifndef __FUNC_CALL__
# define __FUNC_CALL__ memchr
#endif

#include <x86intrin.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NANOSEC_CONVERSION 1E9

char random_char() {
    char c = rand() % 256;
    return c;
}

int main (int argc, char **argv) {
    //seed randomization and inits/decs
    srand(time(NULL));
    int buffer_size = BUFFER_SIZE;
    char* mem_block = (char*) malloc(buffer_size);
    char fill_character = MEM_FILLER;
    char search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;


    //fill memory, set last byte to target
    for(int i = 0; i < buffer_size; i++) {
        *( mem_block + i ) = fill_character;
    }
    *( mem_block + buffer_size - 1) = search_char;

	    for (int j = 0; j < BUFFER_SIZE; j++) {
	      _mm_clflush(mem_block+j);
	    }
    //initial measure
    clock_gettime(CLOCK_MONOTONIC, &start);
    __FUNC_CALL__(mem_block, search_char, buffer_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld\n", elapsed_time);

    //loop measure
    for(int i = 0; i < 10; i++) {
	    for (int j = 0; j < BUFFER_SIZE; j++) {
	      _mm_clflush(mem_block+j);
	    }
        clock_gettime(CLOCK_MONOTONIC, &start);
        __FUNC_CALL__(mem_block, search_char, buffer_size);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
        printf("%ld\n", elapsed_time);
    }

    free(mem_block);
    return elapsed_time;
}
