#ifndef MEMCHR_H_
# define MEMCHR_H_

void *thread_memchr(void *vargp);

void *memchr_glibc(void const *s, int c_in, size_t n);
void *memchr_simple(void const *s, int c_in, size_t n);
void *memchr_avx2(void const *s, int c_in, size_t n);
void *memchr_avx512(void const *s, int c_in, size_t n);
void *memchr_sse(void const *s, int c_in, size_t n);
void *memchr_throughput_only(void const *s, int c_in, size_t n);

typedef void (*thread_callback_t) (void *arg);

void *memchr_avx2_warmup(void const *s, int c_in, size_t n, size_t warmup_length,  thread_callback_t fun,
                  void* arg);

typedef void* (*func_ptr_t)(void const*, int, size_t);
func_ptr_t select_implementation(char *implem_arg)
{
    func_ptr_t ret_val;
    if (strcmp("avx2", implem_arg) == 0) {
       ret_val = memchr_avx2;
    }
    else if (strcmp("sse", implem_arg) == 0) {
       ret_val = memchr_sse;
    }
    else if (strcmp("glibc", implem_arg) == 0) {
       ret_val = memchr_glibc;
    }
    else if (strcmp("avx512", implem_arg) == 0) {
#ifdef __AVX512BW__
       ret_val = memchr_avx512;
#else
       ret_val = memchr_avx2;
       printf("No AVX512 support. Falling back to AVX2.\n");
#endif
    }
    else if (strcmp("simple", implem_arg) == 0) {
       ret_val = memchr_simple;
    }
    else {
       ret_val = memchr_throughput_only;
    }
    return ret_val;
}


#endif

