//Returns time of memchr operation in nanoseconds
//Takes args in order: 1) buffer_size

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1000000007
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
#include "memchr.h"

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

    //measure
    clock_gettime(CLOCK_MONOTONIC, &start);
    MEMCHR_IMPL(mem_block, search_char, buffer_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(mem_block);
    return elapsed_time;
}
