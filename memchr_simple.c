void* MEMCHR_DEF(void const *s, int c, int n) {
    int end = s + n;
    for (const char *i = s; i < end; ++i) {
        if (*i == c)
            return i;
    }
    return NULL;
}
