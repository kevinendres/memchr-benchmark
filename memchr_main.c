//Returns time of memchr operation in nanoseconds
//Takes args in order: 1) buffer_size
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "memchr.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1000000007UL
#endif

#ifndef FILL_CHAR
# define FILL_CHAR 0x42
#endif

#ifndef SEARCH_CHAR
# define SEARCH_CHAR 0x41
#endif

#define NANOSEC_CONVERSION 1000000000UL

char random_char() {
    char c = rand() % 256;
    return c;
}

int main (int argc, char **argv) {
    //seed randomization and inits/decs
    size_t buffer_size = BUFFER_SIZE;
    char* buffer = (char*) aligned_alloc(64, buffer_size);
    char fill_char = FILL_CHAR;
    char search_char = SEARCH_CHAR; 
    struct timespec start, end;
    size_t elapsed_time;


    //fill memory, set last byte to target
    memset(buffer, fill_char, buffer_size);
    *(buffer + buffer_size - 1) = search_char;

    //measure
    clock_gettime(CLOCK_MONOTONIC, &start);
    MEMCHR_IMPL(buffer, search_char, buffer_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(buffer);
    return elapsed_time;
}
