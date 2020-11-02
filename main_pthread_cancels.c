//Spawns multiple threads, each calling memchr on different block of memory

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "memchr.h"

// #ifndef BUFFER_SIZE
// # define BUFFER_SIZE 8000000007UL
// #endif

#ifndef FILL_CHAR
# define FILL_CHAR 0x42
#endif

#ifndef SEARCH_CHAR
# define SEARCH_CHAR 0x41
#endif

#define NANOSEC_CONVERSION 1000000000UL
// #define NUM_THREADS 7

/* GLOBAL */
char *return_val;
char *buffer;
char search_char;
size_t chunk_size;
size_t num_threads;
size_t final_thread;
size_t buffer_size;
char *return_vals[41];
pthread_t tid[41];

/* Prototype */
void *thread_memchr(void *vargp);

int main (int argc, char **argv) {
    /* argument parsing */
    size_t opt;
    while((opt = getopt(argc, argv, "t:d:")) != -1) {
        switch (opt) {
            case 't': num_threads = atol(optarg); break;
            case 'd': buffer_size = atol(optarg); break;
        }
    }
    //inits/decs
    final_thread = num_threads - 1;
    buffer = (char*) aligned_alloc(64, buffer_size);
    char fill_char = FILL_CHAR;
    search_char = SEARCH_CHAR; 
    struct timespec start, end;
    size_t elapsed_time;

    //thread related inits
    long myid[num_threads];
    chunk_size = buffer_size / num_threads;    //each thread does chunk_size work before syncing, except final thread

    //fill memory, set last byte to search_char
    memset(buffer, fill_char, buffer_size);
    *(buffer + buffer_size - 1) = search_char;

    //threading
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < num_threads; i++) {
        myid[i] = i;
        pthread_create(&tid[i], NULL, thread_memchr, &myid[i]);
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

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(buffer);
    exit(0);
}

void *thread_memchr(void *vargp)
{
    long myid = *((long *) vargp);
    size_t local_chunk_size;
    char *local_return_val;
    char *local_buffer = buffer + myid * chunk_size;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (myid == final_thread) {
        local_chunk_size = buffer_size - myid * chunk_size;
    } else { 
        local_chunk_size = chunk_size; 
        }
    local_return_val = MEMCHR_IMPL(local_buffer, search_char, local_chunk_size);
    if (local_return_val != NULL) {
        return_vals[myid] = local_return_val;
        //cancel any thread working in subsequent parts of the buffer
        for (size_t i = myid + 1; i < num_threads; i++) {
            pthread_cancel(tid[i]);
        }
    }
    return NULL;
}