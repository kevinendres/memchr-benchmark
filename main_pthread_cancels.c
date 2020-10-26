//Spawns multiple threads, each calling memchr on different block of memory

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "memchr.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 40000000007 
#endif

#ifndef MEM_FILLER
# define MEM_FILLER 0x42
#endif

#ifndef SEARCH_STR
# define SEARCH_STR 0x41
#endif

#define NANOSEC_CONVERSION 1E9
#define NUM_THREADS 10

/* GLOBAL */
char *return_val;
char *mem_block;
char search_char;
int chunk_size;
int final_thread;
unsigned long long buffer_size;
pthread_t tid[NUM_THREADS];
char *return_vals[NUM_THREADS];

/* Prototype */
void *multi_memchr(void *vargp);

int main (int argc, char **argv) {
    //inits/decs
    printf("begin main\n");
    buffer_size = BUFFER_SIZE;
    final_thread = NUM_THREADS - 1;
    if ((mem_block = (char*) malloc(buffer_size)) == NULL) {
        printf("failed attempt");
        abort();
    }
    char fill_character = MEM_FILLER;
    search_char = SEARCH_STR; 
    struct timespec start, end;
    long elapsed_time;

    //thread related inits
    long myid[NUM_THREADS];
    chunk_size = buffer_size / NUM_THREADS;    //each thread does chunk_size work before syncing, except final thread

    //fill memory, set last byte to search_char
    for (int i = 0; i < buffer_size; i++) {
        *(mem_block + i) = fill_character;
    }
    *(mem_block + buffer_size - 1) = search_char;

    printf("beginning threading\n");
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

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(mem_block);
    exit(0);
}

void *multi_memchr(void *vargp)
{
    long myid = *((long *) vargp);
    int local_chunk_size;
    char *local_return_val;
    char *local_mem_block = mem_block + myid * chunk_size;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (myid == final_thread) {
        local_chunk_size = buffer_size - myid * chunk_size;
    } else { 
        local_chunk_size = chunk_size; 
        }
    local_return_val = MEMCHR_IMPL(local_mem_block, search_char, local_chunk_size);
    if (local_return_val != NULL) {
        return_vals[myid] = local_return_val;
        /* cancel any thread working in subsequent parts of the buffer
            let earlier threads run in case of earlier hit */
        for (int i = myid + 1; i < NUM_THREADS; i++) {
            pthread_cancel(tid[i]);
        }
    }
    return NULL;
}