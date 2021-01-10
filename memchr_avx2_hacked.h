#ifndef MEMCHR_H_
# define MEMCHR_H_

void *MEMCHR_IMPL(void const *s, int c_in, size_t n, size_t warmup_length, size_t *clock, int event_set);

#endif
