#include <stddef.h>

void* __FUNC_CALL__(void const *s, int c, int n) {
    void* end = s + n;
    for (char *i = s; i < end; ++i) {
        if (*i == c)
            return i;
    }
    return NULL;
}
