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
long long counters[401];
sem_t active;
sem_t done;
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
    size_t papi_elapsed_time;
    size_t start_time, end_time;
    pthread_attr_t detach_attr;

    //thread related inits
    long myid[num_threads];
    chunk_size = buffer_size / num_threads;    //each thread does chunk_size work before syncing, except final thread
    pthread_attr_init(&detach_attr);
    pthread_attr_setdetachstate(&detach_attr, PTHREAD_CREATE_DETACHED);
    sem_init(&active, 0, num_threads - 1);
    sem_init(&done, 0, 0);

    //fill memory, set last byte to search_char
    memset(buffer, fill_char, buffer_size);
    *(buffer + buffer_size - 1) = search_char;
    
    //papi inits
    _mm_lfence();
    PAPI_library_init(PAPI_VER_CURRENT);
    start_time = PAPI_get_real_usec();

    //threading
    for (size_t i = 0; i < num_threads; i++) {
        myid[i] = i;
        pthread_create(&tid[i], &detach_attr, thread_memchr, &myid[i]);
    }

    //insert semaphore code for syncing up here
    sem_wait(&done);
    //look for first search hit from memchr
    for (size_t i = 0; i < num_threads; i++) {
        if (return_vals[i] != NULL) {
            return_val = return_vals[i];
            break;
        }
    }
    end_time = PAPI_get_real_usec();
    _mm_lfence();

    /* computer times */
    papi_elapsed_time = end_time - start_time;
    printf("%ld", papi_elapsed_time);

    //Papi timing printouts
    for (size_t i = 0; i < num_threads; ++i) {
        printf(",%ld,%ld,%ld,%ld", thread_start_times[i] - start_time, thread_end_times[i] - thread_start_times[i], thread_end_times[i] - thread_warmedup_times[i], end_time - thread_end_times[i]);
        for (size_t j = i * 10; j < (i + 1) * 10; ++j) {
            printf(",%lld", counters[j]);
        }
    }

    sem_destroy(&active);
    sem_destroy(&done);
    free(buffer);
    exit(0);
}

typedef struct {
    int event_set;
    size_t warmup_start_time;
} thread_info_t;

void thread_memchr_callback (void *arg) {
  thread_info_t* pinfo = arg;
  pinfo->warmup_start_time = PAPI_get_real_usec();
  PAPI_start (pinfo->event_set);
}

void *thread_memchr(void *vargp)
{
    thread_info_t info = {PAPI_NULL, -1};
    PAPI_thread_init(pthread_self);
    PAPI_create_eventset(&(info.event_set));
    PAPI_add_event(info.event_set, PAPI_L1_DCM);
    PAPI_add_event(info.event_set, PAPI_L1_ICM);
    PAPI_add_event(info.event_set, PAPI_L2_DCM);
    PAPI_add_event(info.event_set, PAPI_L2_ICM);
    PAPI_add_event(info.event_set, PAPI_L1_TCM);
    PAPI_add_event(info.event_set, PAPI_L2_TCM);
    PAPI_add_event(info.event_set, PAPI_L3_TCM);
    PAPI_add_event(info.event_set, PAPI_CA_SNP);
    PAPI_add_event(info.event_set, PAPI_CA_SHR);
    PAPI_add_event(info.event_set, PAPI_CA_CLN);
    size_t thread_time = PAPI_get_real_usec();
    long myid = *((long *) vargp);
    thread_start_times[myid] = thread_time;
    size_t local_chunk_size;
    char *local_return_val;
    char *local_buffer = buffer + myid * chunk_size;
    size_t warmup_length;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (myid == final_thread) {
        local_chunk_size = buffer_size - myid * chunk_size;
    } else { 
        local_chunk_size = chunk_size;
        }
    warmup_length = ((local_chunk_size / 10) / 128) * 128;    //ensure that the warmup length is a multiple of 4 * VEC_SIZE
    thread_time = PAPI_get_real_usec();
    local_return_val = MEMCHR_IMPL(local_buffer, search_char, local_chunk_size, warmup_length, &thread_memchr_callback, &info);
    if (local_return_val != NULL) {
        return_vals[myid] = local_return_val;
        //cancel any thread working in subsequent parts of the buffer
        for (size_t i = myid + 1; i < num_threads; i++) {
            pthread_cancel(tid[i]);
        }
    }
    thread_time = PAPI_get_real_usec();
    thread_end_times[myid] = thread_time;
    thread_warmedup_times[myid] = info.warmup_start_time;
    PAPI_read(info.event_set, counters + (myid * 10));
    if (sem_trywait(&active) < 0) {
        sem_post(&done);
    }
    return NULL;
}
