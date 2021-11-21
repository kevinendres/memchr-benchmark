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
#include <libfyaml.h>
#include "memchr.h"
#include "papi_events.h"

#define FILL_CHAR 0x42
#define SEARCH_CHAR 0x41

char *return_val;
char *buffer;
char search_char;
size_t per_thread_buffer_size;
size_t num_threads;
size_t final_thread;
size_t global_buffer_size;
char *return_vals[41];
size_t thread_start_times[41];
size_t thread_warmedup_times[41];
size_t thread_end_times[41];
pthread_t tid[41];
long long counters[401];
atomic_int active;
sem_t done;
char implem_arg[25];
int event_category[10];
char *printable_events[10];
func_ptr_t memchr_implem;
size_t iterations;

void parse(int, char**);
void handle_error(const char*);
void read_config();

int main (int argc, char **argv) {
    char fill_char = FILL_CHAR;
    search_char = SEARCH_CHAR;
    size_t papi_elapsed_time;
    size_t start_time, end_time;
    pthread_attr_t detach_attr;
    char filename[76];

    parse(argc, argv);

    /* allocate buffer */
    buffer = (char*) aligned_alloc(64, global_buffer_size);
    if (buffer == NULL) {
        handle_error("aligned alloc");
    }

    final_thread = num_threads - 1;
    memchr_implem = select_implementation(implem_arg);

    /* create file for writing results */
    time_t epoch_time = time(NULL);
    struct tm *local_time = localtime(&epoch_time);
    sprintf(filename, "%d-%d-%d_%d:%d:%d.csv", local_time->tm_year + 1900, local_time->tm_mon + 1,
            local_time->tm_mday, local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
    FILE *output_file = fopen(filename, "w+");
    if (output_file == NULL) {
        handle_error("file open");
    }

    /* prepare thread-related variables */
    long thread_ids[num_threads];
    per_thread_buffer_size = global_buffer_size / num_threads;
    pthread_attr_init(&detach_attr);
    pthread_attr_setdetachstate(&detach_attr, PTHREAD_CREATE_DETACHED);

    /* fille buffer with junk except for last character */
    memset(buffer, fill_char, global_buffer_size);
    *(buffer + global_buffer_size - 1) = search_char;

    /* start PAPI */
    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
        PAPI_error(1);
    }

    /* main experiment loop */
    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        _mm_lfence();
        atomic_init(&active, num_threads - 1);
        if (sem_init(&done, 0, 0) == -1) {
            handle_error("sem init");
        }

        start_time = PAPI_get_real_usec();
        for (size_t i = 0; i < num_threads; i++) {
            thread_ids[i] = i;
            if(pthread_create(&tid[i], &detach_attr, thread_memchr, &thread_ids[i]) != 0) {
                handle_error("phtread create");
            }
        }

        if (sem_wait(&done) == -1) {
            handle_error("sem wait");
        }

        for (size_t i = 0; i < num_threads; i++) {
            if (return_vals[i] != NULL) {
                return_val = return_vals[i];
                break;
            }
        }

        end_time = PAPI_get_real_usec();
        _mm_lfence();

        papi_elapsed_time = end_time - start_time;

        /* write output to file per experiment iteration */
        for (size_t i = 0; i < num_threads; ++i) {
            fprintf(output_file, "thread %ld,%zu,%ld,%ld,%ld,%ld", i + 1, iteration, papi_elapsed_time,
                thread_start_times[i] - start_time, thread_end_times[i] - thread_start_times[i],
                end_time - thread_end_times[i]);
            for (size_t j = i * 10; j < (i + 1) * 10; ++j) {
                fprintf(output_file, ",%lld", counters[j]);
            }
            fprintf(output_file, "\n");
        }
    }


    if (fclose(output_file) != 0) {
        handle_error("file close");
    }
    if (sem_destroy(&done) == -1) {
        handle_error("sem destroy");
    }
    free(buffer);
    exit(EXIT_SUCCESS);
}

void *thread_memchr(void *vargp)
{
    int event_set = PAPI_NULL;
    if (PAPI_thread_init(pthread_self) != PAPI_OK) {
        PAPI_error(1);
    }
    if (PAPI_create_eventset(&(event_set)) != PAPI_OK) {
        PAPI_error(1);
    }
    load_PAPI_events(&(event_set), event_category);

    size_t local_thread_id = *((size_t *) vargp);
    size_t local_buffer_size;
    char *local_return_val;
    char *local_buffer = buffer + local_thread_id * per_thread_buffer_size;
    if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0) {
        handle_error("pthread set cancel type");
    }
    if (local_thread_id == final_thread) {
        local_buffer_size = global_buffer_size - local_thread_id * per_thread_buffer_size;
    } else {
        local_buffer_size = per_thread_buffer_size;
        }

    size_t thread_start_time = PAPI_get_real_usec();
    if (PAPI_start(event_set) != PAPI_OK) {
        PAPI_error(1);
    }
    local_return_val = memchr_implem(local_buffer, search_char, local_buffer_size);
    if (local_return_val != NULL) {
        return_vals[local_thread_id] = local_return_val;
        for (size_t i = local_thread_id + 1; i < num_threads; i++) {
            pthread_cancel(tid[i]);
        }
    }

    size_t thread_end_time = PAPI_get_real_usec();
    thread_start_times[local_thread_id] = thread_start_time;
    thread_end_times[local_thread_id] = thread_end_time;
    if (PAPI_read(event_set, counters + (local_thread_id * 10)) != PAPI_OK) {
        PAPI_error(1);
    }
    if (atomic_fetch_sub(&active, 1) == 0) {
        sem_post(&done);
    }
    return NULL;
}

void print_help_message()
{
    printf("Memchr benchmarking.\n\nOptions:\n"
    " -c\t\t\t\tread options from config.yaml instead of command line\n"
    " -t <threadcount>\t\tto spawn <threadcount> threads\n"
    " -b <buffersize>\t\tallocate a <buffersize> buffer to search\n"
    " -m <memchrimplementation>\tselect <memchrimplementation>\n"
    " -i <iterations>\t\trun benchmark <iterations> times\n"
    " -e <events>\t\t\tchoose PAPI hardware counter <events>\n"
    " -h\t\t\t\tdisplay this help\n");
}

void read_config()
{
    char PAPI_event[25];
    struct fy_document *config = fy_document_build_from_file(NULL, "../src/config.yaml");
    if (!config) {
        handle_error("config open");
    }
    int count = fy_document_scanf(config,
                                  "/number_of_threads %zu "
                                  "/number_of_iterations %zu "
                                  "/buffer_size %zu "
                                  "/memchr_implementation %s "
                                  "/PAPI_events %s", &num_threads, &iterations, &global_buffer_size,
                                  implem_arg, PAPI_event);
    if (count != 5) {
        handle_error("config scan");
    }
    choose_event_category(strdup(PAPI_event), event_category);
}

void parse(int argc, char** argv)
{
    if (!(argc == 2 || argc == 11)) {
        printf("Please supply command line arguments or use -c for config file\n");
        print_help_message();
        exit(EXIT_FAILURE);
    }

    int opt;
    while((opt = getopt(argc, argv, "ct:b:m:i:e:h")) != -1) {
        switch (opt) {
            case 'c': read_config(); break;
            case 't': num_threads = atol(optarg); break;
            case 'b': global_buffer_size = atol(optarg); break;
            case 'm': strcpy(implem_arg, optarg); break;
            case 'i': iterations = atol(optarg); break;
            case 'e': choose_event_category(optarg, event_category); break;
            case 'h':
            default: print_help_message();
                exit(EXIT_FAILURE);
        }
    }
}

void handle_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
