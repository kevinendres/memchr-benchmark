#include <stddef.h>
#include "memchr.h"

void* MEMCHR_IMPL(const void *s, int c_in, size_t n) {
    const char* end = (char*) s + n;
    char c = (unsigned char) c_in;
    for (char* i = s; i < end; ++i) {
        if (*i == c)
            return i;
    }
    return NULL;
}
