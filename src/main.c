
//Spawns multiple threads, each calling memchr on different block of memory

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
#include <stdatomic.h>
#include <semaphore.h>
#include "memchr.h"
#include "papi_events.h"

#define FILL_CHAR 0x42
#define SEARCH_CHAR 0x41

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
atomic_int active;
sem_t done;
char *implem_arg;
int event_category[10];
func_ptr_t memchr_implem;

int main (int argc, char **argv) {
    /* argument parsing */
    int opt;
    while((opt = getopt(argc, argv, "t:d:i:p:")) != -1) {
        switch (opt) {
            case 't': num_threads = atol(optarg); break;
            case 'd': buffer_size = atol(optarg); break;
            case 'i': implem_arg = strdup(optarg); break;
            case 'p': choose_event_category(optarg, event_category); break;
        }
    }

    memchr_implem = select_implementation(implem_arg);

    //inits/decs
    final_thread = num_threads - 1;
    buffer = (char*) aligned_alloc(64, buffer_size);
    char fill_char = FILL_CHAR;
    search_char = SEARCH_CHAR;
    size_t papi_elapsed_time;
    size_t start_time, end_time;
    pthread_attr_t detach_attr;
    int procid = getpid();

    //thread related inits
    long myid[num_threads];
    chunk_size = buffer_size / num_threads;    //each thread does chunk_size work, except final thread
    pthread_attr_init(&detach_attr);
    pthread_attr_setdetachstate(&detach_attr, PTHREAD_CREATE_DETACHED);
    atomic_init(&active, num_threads - 1);
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

    //sync
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
    printf("main,%d,%ld\n", procid, papi_elapsed_time);

    //Papi timing printouts
    for (size_t i = 0; i < num_threads; ++i) {
        printf("thread %ld,%d,%ld,%ld,%ld,%ld", i + 1, procid, papi_elapsed_time,
               thread_start_times[i] - start_time, thread_end_times[i] - thread_start_times[i],
               end_time - thread_end_times[i]);
        for (size_t j = i * 10; j < (i + 1) * 10; ++j) {
            printf(",%lld", counters[j]);
        }
        printf("\n");
    }

    sem_destroy(&done);
    free(buffer);
    exit(0);
}

void *thread_memchr(void *vargp)
{
    //prepare PAPI events
    int event_set = PAPI_NULL;
    PAPI_thread_init(pthread_self);
    PAPI_create_eventset(&(event_set));
    load_PAPI_events(&(event_set), event_category);

    //local initializations; record start time
    size_t myid = *((size_t *) vargp);
    size_t thread_time = PAPI_get_real_usec();
    thread_start_times[myid] = thread_time;
    size_t local_chunk_size;
    char *local_return_val;
    char *local_buffer = buffer + myid * chunk_size;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (myid == final_thread) {
        local_chunk_size = buffer_size - myid * chunk_size;
    } else {
        local_chunk_size = chunk_size;
        }

    //run memchr
    PAPI_start(event_set);
    local_return_val = memchr_implem(local_buffer, search_char, local_chunk_size);
    if (local_return_val != NULL) {
        return_vals[myid] = local_return_val;
        //cancel any thread working in subsequent parts of the buffer
        for (size_t i = myid + 1; i < num_threads; i++) {
            pthread_cancel(tid[i]);
        }
    }

    //process times/locks
    thread_time = PAPI_get_real_usec();
    thread_end_times[myid] = thread_time;
    PAPI_read(event_set, counters + (myid * 10));
    if (atomic_fetch_sub(&active, 1) == 0) {
        sem_post(&done);
    }
    return NULL;
}
