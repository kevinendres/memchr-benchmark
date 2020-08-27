//Spawns multiple threads, each calling memchr on different block of memory

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "memchr.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 1000000007 
#endif

#ifndef MEM_FILLER
# define MEM_FILLER 0x42
#endif

#ifndef SEARCH_STR
# define SEARCH_STR 0x41
#endif

#define NANOSEC_CONVERSION 1E9
#define NUM_THREADS 10
#define PAGE_SIZE sysconf(_SC_PAGESIZE) 

/* GLOBAL */
char *return_val;
char *mem_block;
char search_char;
int chunk_size;
int final_thread;
int buffer_size;
pthread_t tid[NUM_THREADS];

/* Prototype */
void *multi_memchr(void *vargp);

int main (int argc, char **argv) {
    //inits/decs
    buffer_size = BUFFER_SIZE;
    int final_thread = NUM_THREADS - 1;
    mem_block = (char*) aligned_alloc(64, buffer_size);
    char fill_character = MEM_FILLER;
    search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;

    //thread related inits
    long myid[NUM_THREADS];
    chunk_size = buffer_size / NUM_THREADS;    //each thread does chunk_size work before syncing, except final thread
    //char *results[NUM_THREADS];

    //fill memory, set last byte to search_char
    for(int i = 0; i < buffer_size; i++) {
        *( mem_block + i ) = fill_character;
    }
    *( mem_block + buffer_size - 1) = search_char;

    //threading
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) {
        myid[i] = i;
        pthread_create(&tid[i], NULL, multi_memchr, &myid[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tid[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld\n", elapsed_time);
    printf("number of threads: %d\n", NUM_THREADS);

    free(mem_block);
    exit(0);
}

void *multi_memchr(void *vargp)
{
    long myid = *((long *) vargp);
    int local_chunk_size;
    char *local_return_val;
    char *local_mem_block = mem_block + myid * chunk_size;
//    printf("this threads id is %d\n", myid);
    if (myid == final_thread) {
        local_chunk_size = buffer_size - myid * chunk_size;
    } else { 
        local_chunk_size = chunk_size; 
        }
    local_return_val = memchr(local_mem_block, search_char, local_chunk_size);
    if (local_return_val != NULL) {
        return_val = local_return_val;
        for (int i = 0; i < NUM_THREADS; i++) {
            if (i != myid) {
                pthread_cancel(tid[i]);
            }
        }
    }
    return NULL;
}