//Returns time of memchr operation in nanoseconds
//Takes args in order: 1) buffer_size

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
    int buffer_size = argv[1];
    char* mem_block = (char*) malloc(buffer_size);
    char character;
    char search_char = random_char(); 
    struct timespec start, end;
    long elapsed_time;


    //fill memory with random characters
    for(int i = 0; i < buffer_size; i++) {
        character = random_char();
        *( mem_block + i ) = character;
    }

    //measure
    clock_gettime(CLOCK_MONOTONIC, &start);
    memchr(mem_block, search_char, buffer_size);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);

    free(mem_block);
    return elapsed_time;
}