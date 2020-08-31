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
#define L3_SIZE sysconf(_SC_LEVEL3_CACHE_SIZE)

/* GLOBAL */
char *mem_block;
char search_char;
int block_size;
int slice_size;
int final_thread;
int buffer_size;
char *return_vals[NUM_THREADS];
pthread_barrier_t sync_point;

/* Prototype */
void *multi_memchr(void *vargp);

int main(int argc, char **argv) {
    //inits/decs
    buffer_size = BUFFER_SIZE;
    final_thread = NUM_THREADS - 1;
    mem_block = (char*) aligned_alloc(64, buffer_size);
    //preserve starting pointer to free later
    char *mem_clear = mem_block;
    char fill_character = MEM_FILLER;
    search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;
    char *return_val = NULL;
    pthread_t tid[NUM_THREADS];

    //thread related inits
    long myid[NUM_THREADS];
    block_size = L3_SIZE;
    slice_size = L3_SIZE / NUM_THREADS;    //each thread does slice_size work before syncing, except final thread
    pthread_barrier_init(&sync_point, NULL, NUM_THREADS);

    //fill memory, set last byte to search_char
    for (int i = 0; i < buffer_size; i++) {
        *(mem_block + i) = fill_character;
    }
    *(mem_block + buffer_size - 1) = search_char;

    //threading
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < NUM_THREADS; i++) {
        myid[i] = i;
        pthread_create(&tid[i], NULL, multi_memchr, &myid[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tid[i], NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        if (return_vals[i] != NULL) {
            return_val = return_vals[i];
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    pthread_barrier_destroy(&sync_point);

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(mem_clear);
    exit(0);
}

void *multi_memchr(void *vargp)
{
    long myid = *((long *) vargp);
    int local_block_size = block_size;
    int local_slice_size = slice_size;
    char *local_return_val;
    char *local_start;
    char *global_start = mem_block;
    int remaining_bytes = buffer_size;
    for (int i = 0; remaining_bytes > 0 ; i++) {
        //set starting point
        local_start = global_start + local_block_size * i + myid * local_slice_size; //need to calculate local_slice_size afterwards? what about final iteration??
        
        //assign remainder of block to final thread
        if (myid == final_thread)
            local_slice_size = (remaining_bytes < L3_SIZE ? remaining_bytes : L3_SIZE) - myid * slice_size;

        local_return_val = memchr(local_start, search_char, local_slice_size);

        // store result in an array to avoid races 
        return_vals[myid] = local_return_val;
        
        //sync and check for hits
        pthread_barrier_wait(&sync_point);
        for (int j = 0; j < NUM_THREADS; j++) {
            if (return_vals[j] != NULL) {
                return NULL;
            }
        }

        //update sizes if last block of memory is smaller than L3
        remaining_bytes = buffer_size - (i + 1) * block_size;
        if (remaining_bytes < L3_SIZE) {
            local_slice_size = remaining_bytes / NUM_THREADS;
        }
    }
    return NULL;
}