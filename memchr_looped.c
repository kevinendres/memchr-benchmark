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

#ifndef __FUNC_CALL__
# define __FUNC_CALL__ memchr
#endif

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
    char* end_mem_block = mem_block + buffer_size;
    char search_char = SEARCH_STR;
    char fill_char = MEM_FILLER; 
    struct timespec start, end;
    long elapsed_time;


    //fill memory, set last byte to target
    for(int i = 0; i < buffer_size; i++) {
        *( mem_block + i ) = fill_char;
    }
    *( mem_block + buffer_size / 2 ) = search_char;
    *( mem_block + buffer_size - 1) = search_char;

    //measure
    while ( buffer_size > 0 ) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        char* next_start = memchr(mem_block, search_char, buffer_size);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
        printf("elapsed time: %ld\n", elapsed_time);
        buffer_size = buffer_size - ( next_start - mem_block );
        printf("buffer decrement %d", next_start - mem_block );
        printf("buffer size: %d", buffer_size);
        mem_block = (char*) next_start;
    }

    free(mem_block);
    return elapsed_time;
}
