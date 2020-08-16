//Returns time of memchr operation in nanoseconds
//Takes args in order: 1) buffer_size

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "memchr.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1000000000 //1073741824 //1 GiB
#endif

#ifndef MEM_FILLER
# define MEM_FILLER 0x42
#endif

#ifndef SEARCH_STR
# define SEARCH_STR 0x41
#endif

// #ifndef MEMCHR_IMPL
// # define MEMCHR_IMPL memchr
// #endif

#define NANOSEC_CONVERSION 1E9

int main (int argc, char **argv) {
    //inits/decs
    int buffer_size = BUFFER_SIZE;
    char* mem_block = (char*) aligned_alloc(128, buffer_size);
    char fill_character = MEM_FILLER;
    char search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;
    char* return_val;

    //fill memory, set last byte to target
    for(int i = 0; i < buffer_size; i++) {
        *( mem_block + i ) = fill_character;
    }
    *( mem_block + buffer_size - 1) = search_char;

    //measure
    clock_gettime(CLOCK_MONOTONIC, &start);
    MEMCHR_IMPL(mem_block, search_char, buffer_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    return_val = (char *) MEMCHR_IMPL(mem_block, search_char, buffer_size);
    printf("%ld\n", elapsed_time);
    printf("value at end of buffer: %x\n", *(mem_block + buffer_size -1));
    printf("return value from func: %x\n", *(return_val));

    free(mem_block);
    exit(0);
}
