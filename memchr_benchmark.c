//Returns time of memchr operation in nanoseconds

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NANOSEC_CONVERSION 1E9

char random_char() {
    char c = rand() % 256;
    return c;
}

int main () {
    //seed randomization and inits/decs
    srand(time(NULL));
    char* mem_block = (char*) malloc(1000);
    char character;
    char search_char = random_char(); 
    struct timespec start, end;
    long elapsed_time;

    //fill memory with random characters
    for(int i = 0; i < 1000; i++) {
        character = random_char();
        *( mem_block + i ) = character;
    }

    //measure
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    memchr(mem_block, search_char, 1000);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);

    free(mem_block);
    return elapsed_time;
}