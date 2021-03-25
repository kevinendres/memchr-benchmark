//Spawns multiple threads, each calling memchr on different block of memory
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "memchr.h"

// #ifndef BUFFER_SIZE
// # define BUFFER_SIZE 1000000007UL 
// #endif

#ifndef FILL_CHAR
# define FILL_CHAR 0x42
#endif

#ifndef SEARCH_CHAR
# define SEARCH_CHAR 0x41
#endif

#define NANOSEC_CONVERSION 1000000000UL
// #define NUM_THREADS 20
// #define L3_SIZE sysconf(_SC_LEVEL3_CACHE_SIZE)

// #ifndef BLOCK_SIZE
// # define BLOCK_SIZE L3_SIZE
// #endif

/* GLOBALS */
char *buffer;
char search_char;
size_t block_size;
size_t slice_size;
size_t num_threads;
size_t final_thread;       /* for handling remainders of uneven block sizes*/
size_t buffer_size;
char *return_vals[41];
pthread_barrier_t sync_point;

/* Prototype */
void *thread_memchr(void *vargp);

int main(int argc, char **argv) {
    /* argument parsing */
    size_t opt;
    while((opt = getopt(argc, argv, "t:d:b:")) != -1) {
        switch (opt) {
            case 't': num_threads = atol(optarg); break;
            case 'd': buffer_size = atol(optarg); break;
            case 'b': block_size = atol(optarg); break;
        }
    }
    /* inits/declarations */
    final_thread = num_threads - 1;
    buffer = (char*) aligned_alloc(64, buffer_size);
    char fill_char = FILL_CHAR;
    search_char = SEARCH_CHAR; 
    struct timespec start, end;
    size_t elapsed_time;
    char *return_val = NULL;
    pthread_t tid[num_threads];
    struct rusage total_usage;
    struct rusage start_usage;
    struct rusage end_usage;

    /* thread related inits */
    long my_id[num_threads];
    slice_size = block_size / num_threads;    /* each thread does slice_size work before syncing, except final thread */
    pthread_barrier_init(&sync_point, NULL, num_threads);

    /* fill memory, set last byte to search_char */
    memset(buffer, fill_char, buffer_size);
    *(buffer + buffer_size - 1) = search_char;

    /* spawn and join threads with timer */
    getrusage(RUSAGE_SELF, &start_usage);
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < num_threads; i++) {
        my_id[i] = i;
        pthread_create(&tid[i], NULL, thread_memchr, &my_id[i]);
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
    if (getrusage(RUSAGE_SELF, &end_usage) == 0) {
        printf("memchr call stats\n");
        printf("voluntary context switches: %ld\n", end_usage.ru_nvcsw - start_usage.ru_nvcsw);
        printf("involuntary context switches: %ld\n", end_usage.ru_nivcsw - start_usage.ru_nivcsw);
    }
    pthread_barrier_destroy(&sync_point);

    elapsed_time = (end.tv_sec * NANOSEC_CONVERSION + end.tv_nsec) - (start.tv_sec * NANOSEC_CONVERSION + start.tv_nsec);
    printf("%ld", elapsed_time);

    free(buffer);
    if (getrusage(RUSAGE_SELF, &total_usage) == 0) {
        printf("ending program\n");
        printf("total voluntary context switches: %ld\n", total_usage.ru_nvcsw);
        printf("total involuntary context switches: %ld\n", total_usage.ru_nivcsw);
    }
    exit(0);
}

void *thread_memchr(void *vargp)
{
    long myid = *((long *) vargp);
    size_t local_block_size = block_size;
    size_t local_slice_size = slice_size;
    char *local_return_val;
    char *global_start = buffer;
    char *local_start = global_start;
    size_t remaining_bytes = buffer_size;
    for (size_t i = 0; (signed long) remaining_bytes > 0 ; i++) {
        //set starting 
        local_start = global_start + local_block_size * i + myid * local_slice_size; //need to calculate local_slice_size afterwards? what about final iteration??

        //assign remainder of block to final thread
        if (myid == final_thread)
            local_slice_size = (remaining_bytes < block_size ? remaining_bytes : block_size) - myid * slice_size;
        local_return_val = MEMCHR_IMPL(local_start, search_char, local_slice_size);

        //sync and check for hits
        pthread_barrier_wait(&sync_point);
        return_vals[myid] = local_return_val;
        for (int j = 0; j < num_threads; j++) {
            if (return_vals[j] != NULL) {
                return NULL;
            }
        }

        //update sizes if last block of memory is smaller than L3
        remaining_bytes = buffer_size - (i + 1) * block_size;
        if (remaining_bytes < block_size) {
            local_slice_size = remaining_bytes / num_threads;
        }
    }
    return NULL;
}