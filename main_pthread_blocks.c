//Spawns multiple threads, each calling memchr on different block of memory
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>

#include "memchr.h"

// #ifndef BUFFER_SIZE
// # define BUFFER_SIZE 1000000007 
// #endif

#ifndef MEM_FILLER
# define MEM_FILLER 0x42
#endif

#ifndef SEARCH_STR
# define SEARCH_STR 0x41
#endif

#define NANOSEC_CONVERSION 1E9
// #define NUM_THREADS 20
// #define L3_SIZE sysconf(_SC_LEVEL3_CACHE_SIZE)

// #ifndef BLOCK_SIZE
// # define BLOCK_SIZE L3_SIZE
// #endif

/* GLOBALS */
char *mem_block;
char search_char;
int block_size;
int slice_size;
int num_threads;
int final_thread;       /* for handling remainders of uneven block sizes*/
int buffer_size;
char *return_vals;
pthread_barrier_t sync_point;

/* Prototype */
void *multi_memchr(void *vargp);

int main(int argc, char **argv) {
    /* argument parsing */
    int opt;
    while((opt = getopt(argc, argv, "t:d:b:")) != -1) {
        switch (opt) {
            case 't': num_threads = atoi(optarg); break;
            case 'd': buffer_size = atoi(optarg); break;
            case 'b': block_size = atoi(optarg); break;
        }
    }
    /* inits/declarations */
    return_vals = malloc(num_threads);
    final_thread = num_threads - 1;
    mem_block = (char*) aligned_alloc(64, buffer_size);
    char fill_character = MEM_FILLER;
    search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;
    char *return_val = NULL;
    pthread_t tid[num_threads];

    /* thread related inits */
    long my_id[num_threads];
    slice_size = block_size / num_threads;    /* each thread does slice_size work before syncing, except final thread */
    pthread_barrier_init(&sync_point, NULL, num_threads);

    /* fill memory, set last byte to search_char */
    for (int i = 0; i < buffer_size; i++) {
        *(mem_block + i) = fill_character;
    }
    *(mem_block + buffer_size - 1) = search_char;

    /* spawn and join threads with timer */
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < num_threads; i++) {
        my_id[i] = i;
        pthread_create(&tid[i], NULL, multi_memchr, &my_id[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(tid[i], NULL);
    }
    for (int i = 0; i < num_threads; i++) {
        if (return_vals[i] != NULL) {
            return_val = return_vals[i];
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    pthread_barrier_destroy(&sync_point);

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(mem_block);
    free(return_vals)
    exit(0);
}

void *multi_memchr(void *vargp)
{
    /* inits/declarations */
    long my_id = *((long *) vargp);
    int local_block_size = block_size;
    int local_slice_size = slice_size;
    char *local_return_val;
    char *local_start = mem_block;
    int remaining_bytes = buffer_size;
    for (int i = 0; remaining_bytes > 0 ; i++) {
        /* starting point for current thread in current block*/
        local_start += (local_block_size * i) + (my_id * local_slice_size); //need to calculate local_slice_size afterwards? what about final iteration??
        
        /* assign remainder of block to final thread */
        if (my_id == final_thread) {
            local_slice_size = local_block_size - (my_id * slice_size);
        }
        local_return_val = MEMCHR_IMPL(local_start, search_char, local_slice_size);

        //sync and check for hits
        pthread_barrier_wait(&sync_point);
        return_vals[my_id] = local_return_val;
        for (int j = 0; j < num_threads; j++) {
            if (return_vals[j] != NULL) {
                return NULL;
            }
        }

        /* loop maintenance */
        remaining_bytes = buffer_size - (i + 1) * block_size;
        
        /* update sizes if last block of memory is smaller than the block_size */
        if (remaining_bytes < block_size) {
            local_block_size = remaining_bytes;
            local_slice_size = local_block_size / num_threads;
        }
    }
    return NULL;
}
