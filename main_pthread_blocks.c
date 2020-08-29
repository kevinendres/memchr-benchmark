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
#define CACHELINE_SIZE sysconf(_SC_LEVEL1_DCACHE_LINESIZE) 
#define L3_SIZE sysconf(_SC_LEVEL3_CACHE_SIZE)

/* GLOBAL */
char *return_val;
char *mem_block;
char search_char;
int chunk_size;
int final_thread;
int buffer_size;
struct return_val_storage {
    char* thread_result;
    char padding[CACHELINE_SIZE - sizeof(char*)];
} return_vals[NUM_THREADS];

/* Prototype */
void *multi_memchr(void *vargp);

int main(int argc, char **argv) {
    //inits/decs
    buffer_size = BUFFER_SIZE;
    final_thread = NUM_THREADS - 1;
    mem_block = (char*) aligned_alloc(64, buffer_size);
    char *mem_clear = mem_block;
    char fill_character = MEM_FILLER;
    search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;
    return_val = NULL;
    pthread_t tid[NUM_THREADS];
    

    //thread related inits
    long myid[NUM_THREADS];
    chunk_size = L3_SIZE / NUM_THREADS;    //each thread does chunk_size work before syncing, except final thread
    //char *results[NUM_THREADS];

    //fill memory, set last byte to search_char
    for (int i = 0; i < buffer_size; i++) {
        *(mem_block + i) = fill_character;
    }
    *(mem_block + L3_SIZE + 7) = search_char;

    //threading
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (buffer_size > 0) {
        for (int i = 0; i < NUM_THREADS; i++) {
            myid[i] = i;
            pthread_create(&tid[i], NULL, multi_memchr, &myid[i]);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(tid[i], NULL);
        }
        if (return_val != NULL) 
            break;
        buffer_size -= L3_SIZE;
        mem_block += L3_SIZE;
        if (buffer_size < L3_SIZE) 
            chunk_size = buffer_size / NUM_THREADS;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("total elapsed time: %ld\n", elapsed_time);
    printf("number of threads: %d\n", NUM_THREADS);

    free(mem_clear);
    exit(0);
}

void *multi_memchr(void *vargp)
{
    long myid = *((long *) vargp);
    int local_chunk_size;
    char *local_return_val;
    char *local_mem_block = mem_block + myid * chunk_size;
    if (myid == final_thread) {
        local_chunk_size = (buffer_size < L3_SIZE ? buffer_size : L3_SIZE) - myid * chunk_size;
    } else { 
        local_chunk_size = chunk_size; 
        }
    local_return_val = memchr(local_mem_block, search_char, local_chunk_size);
    if (local_return_val != NULL)
        return_val = local_return_val;
    return NULL;
}