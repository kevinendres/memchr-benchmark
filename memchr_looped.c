//Returns time of memchr operation in nanoseconds
//Takes args in order: 1) buffer_size

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1000000
#endif

#ifndef MEM_FILLER
# define MEM_FILLER 0x42
#endif

#ifndef SEARCH_STR
# define SEARCH_STR 0x41
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NANOSEC_CONVERSION 1E9

int main (int argc, char **argv) {
    //inits/decs
    int buffer_size = BUFFER_SIZE;
    char* mem_block = (char*) malloc(buffer_size);
    char* end_mem_block = mem_block + buffer_size;
    char search_char = SEARCH_STR;
    char fill_char = MEM_FILLER; 
    struct timespec start, end;
    long elapsed_time;
    char* next_start;


    //fill memory, set last byte to target
    for(int i = 0; i < buffer_size; i++) {
        *( mem_block + i ) = fill_char;
    }
    *( mem_block + buffer_size / 2 ) = search_char;
    *( mem_block + buffer_size - 1) = search_char;
    *( mem_block + buffer_size - 10) = search_char;

    //measure
    while ( buffer_size > 1 ) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        memchr(mem_block, search_char, buffer_size);
        clock_gettime(CLOCK_MONOTONIC, &end);
        next_start = memchr(mem_block, search_char, buffer_size);
        elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
        printf("elapsed time: %ld\n", elapsed_time);
        buffer_size = end_mem_block - next_start;
        mem_block = (char *) next_start + 1;
    }

    free(mem_block);
    return elapsed_time;
}
