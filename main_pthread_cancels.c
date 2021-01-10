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
#include <semaphore.h>
#include "memchr_avx2_hacked.h"

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
size_t thread_start_times[41];
size_t thread_warmedup_times[41];
size_t thread_end_times[41];
pthread_t tid[41];
sem_t join;
int event_set = PAPI_NULL;

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
    size_t papi_elapsed_time, r_user_elapsed_time, r_kernel_elapsed_time, max_res_set;
    size_t soft_page_faults, hard_page_faults, IO_in_calls, IO_out_calls, vol_con_switch, invol_con_switch; 
    struct rusage start_usage, end_usage;
    size_t start_time, end_time;
    pthread_attr_t detach_attr;

    //thread related inits
    long myid[num_threads];
    chunk_size = buffer_size / num_threads;    //each thread does chunk_size work before syncing, except final thread
    pthread_attr_init(&detach_attr);
    pthread_attr_setdetachstate(&detach_attr, PTHREAD_CREATE_DETACHED);
    sem_init(&join, 0, 1);

    //fill memory, set last byte to search_char
    memset(buffer, fill_char, buffer_size);
    *(buffer + buffer_size - 1) = search_char;
    
    //papi inits
    _mm_lfence();
    PAPI_library_init(PAPI_VER_CURRENT);
    PAPI_create_eventset(&event_set);
    PAPI_add_event(event_set, PAPI_L1_DCM);
    PAPI_add_event(event_set, PAPI_L1_ICM);
    PAPI_add_event(event_set, PAPI_L2_DCM);
    PAPI_add_event(event_set, PAPI_L2_ICM);
    PAPI_add_event(event_set, PAPI_L1_TCM);
    PAPI_add_event(event_set, PAPI_L2_TCM);
    PAPI_add_event(event_set, PAPI_L3_TCM);
    PAPI_add_event(event_set, PAPI_CA_SNP);
    PAPI_add_event(event_set, PAPI_CA_SHR);
    PAPI_add_event(event_set, PAPI_CA_CLN);
    getrusage(RUSAGE_SELF, &start_usage);
    start_time = PAPI_get_real_usec();

    //threading
    for (int i = 0; i < num_threads; i++) {
        myid[i] = i;
        pthread_create(&tid[i], &detach_attr, thread_memchr, &myid[i]);
    }

    //insert semaphore code for syncing up here
    for (int i = 0; i < num_threads; i++) {
        pthread_join()
    }
    for (int i = 0; i < num_threads; i++) {
        if (return_vals[i] != NULL) {
            return_val = return_vals[i];
            break;
        }
    }
    end_time = PAPI_get_real_usec();
    getrusage(RUSAGE_SELF, &end_usage);
    _mm_lfence();

    /* computer rusage values */
    papi_elapsed_time = end_time - start_time;
    r_user_elapsed_time = (end_usage.ru_utime.tv_sec * 1000000 + end_usage.ru_utime.tv_usec) - (start_usage.ru_utime.tv_sec * 1000000 + start_usage.ru_utime.tv_usec);
    r_kernel_elapsed_time = (end_usage.ru_stime.tv_sec * 1000000 + end_usage.ru_stime.tv_usec) - (start_usage.ru_stime.tv_sec * 1000000 + start_usage.ru_stime.tv_usec);
    max_res_set = end_usage.ru_maxrss - start_usage.ru_maxrss;
    soft_page_faults = end_usage.ru_minflt - start_usage.ru_minflt;
    hard_page_faults = end_usage.ru_majflt - start_usage.ru_majflt;
    IO_in_calls = end_usage.ru_inblock - start_usage.ru_inblock;
    IO_out_calls = end_usage.ru_oublock - start_usage.ru_oublock;
    vol_con_switch = end_usage.ru_nvcsw - start_usage.ru_nvcsw;
    invol_con_switch = end_usage.ru_nivcsw - start_usage.ru_nivcsw;
    printf("%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld", papi_elapsed_time, r_user_elapsed_time, r_kernel_elapsed_time, max_res_set, soft_page_faults, hard_page_faults,
        IO_in_calls, IO_out_calls, vol_con_switch, invol_con_switch);
    for (int i = 0; i < num_threads; ++i) {
        printf(",%ld,%ld,%ld", thread_start_times[i] - start_time, thread_end_times[i] - thread_start_times[i], end_time - thread_end_times[i]);
    }

    sem_destroy(&join);
    free(buffer);
    exit(0);
}

void *thread_memchr(void *vargp)
{
    size_t thread_time = PAPI_get_real_usec();
    long myid = *((long *) vargp);
    thread_start_times[myid] = thread_time;
    size_t local_chunk_size;
    char *local_return_val;
    char *local_buffer = buffer + myid * chunk_size;
    size_t warmup_start_time;
    size_t warmup_length;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (myid == final_thread) {
        local_chunk_size = buffer_size - myid * chunk_size;
    } else { 
        local_chunk_size = chunk_size; 
        }
    warmup_length = ((local_chunk_size / 10) / 128) * 128;    //ensure that the warmup length is a multiple of 4 * VEC_SIZE
    local_return_val = MEMCHR_IMPL(local_buffer, search_char, local_chunk_size, warmup_length, &warmup_start_time, event_set);
    if (local_return_val != NULL) {
        return_vals[myid] = local_return_val;
        //cancel any thread working in subsequent parts of the buffer
        for (size_t i = myid + 1; i < num_threads; i++) {
            pthread_cancel(tid[i]);
        }
    }
    thread_time = PAPI_get_real_usec();
    thread_end_times[myid] = thread_time;
    thread_warmedup_times[myid] = warmup_start_time;
    return NULL;
}
