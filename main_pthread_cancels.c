//Spawns multiple threads, each calling memchr on different block of memory

//#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <papi.h>
#include <emmintrin.h>
#include "memchr.h"

#ifndef FILL_CHAR
# define FILL_CHAR 0x42
#endif

#ifndef SEARCH_CHAR
# define SEARCH_CHAR 0x41
#endif

#define NANOSEC_CONVERSION 1000000000UL

/* GLOBAL */
char *return_val;
char *buffer;
char search_char;
size_t chunk_size;
size_t num_threads;
size_t final_thread;
size_t buffer_size;
char *return_vals[41];
size_t thread_times[41];
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
    size_t papi_elapsed_time, thread_total_elapsed_time, r_user_elapsed_time, r_kernel_elapsed_time, max_res_set;
    size_t soft_page_faults, hard_page_faults, IO_in_calls, IO_out_calls, vol_con_switch, invol_con_switch; 
    struct rusage total_usage, start_usage, end_usage;
    size_t start_time, end_time, start_cyc, end_cyc;

    //thread related inits
    long myid[num_threads];
    chunk_size = buffer_size / num_threads;    //each thread does chunk_size work before syncing, except final thread

    //fill memory, set last byte to search_char
    memset(buffer, fill_char, buffer_size);
    *(buffer + buffer_size - 1) = search_char;
    
    //papi inits
    _mm_lfence();
    PAPI_library_init(PAPI_VER_CURRENT);
    PAPI_thread_init(pthread_self);
    PAPI_hl_region_begin("main");
    start_time = PAPI_get_real_usec();

    //threading
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
    end_time = PAPI_get_real_usec();
    PAPI_hl_region_end("main");
    _mm_lfence();

    papi_elapsed_time = end_time - start_time;
    printf("%ld", papi_elapsed_time);

    free(buffer);
    exit(0);
}

void *thread_memchr(void *vargp)
{
    PAPI_hl_region_begin("threads");
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
    PAPI_hl_region_end("threads");
    return NULL;
}
