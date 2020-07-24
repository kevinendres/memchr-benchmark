CC = gcc
CFLAGS = -Wall -O3
CFLAGS += -march=native
#CFLAGS += -march=icelake-client
#CFLAGS += -Wno-unused-function -Wno-parentheses
AVX = -D__FUNC_CALL__="__memchr_avx2"
SSE = -D__FUNC_CALL__="__memchr_sse"

all : memchr_sse memchr_avx2 memchr_simple
.PHONY : all

memchr_sse: memchr_sse.o memchr_main.c
	$(CC) $(CFLAGS) -o memchr_sse memchr_sse.o memchr_main.c $(SSE)

memchr_sse.o : memchr_sse.S
	$(CC) $(CFLAGS) -c memchr_sse.S -I ~/src/glibc/sysdeps/x86_64 -I ~/src/glibc/sysdeps/x86_64/x32 -I ~/src/glibc/include -I ~/src/glibc/build/ -I ~/src/glibc

memchr_avx2: memchr_avx2.o memchr_main.c
	$(CC) $(CFLAGS) -o memchr_avx2 memchr-avx2.o memchr_main.c $(AVX) 

memchr_avx2.o : memchr_avx2.S
	$(CC) $(CFLAGS) -c memchr_avx2.S -I ~/src/glibc/sysdeps/x86_64 -I ~/src/glibc/sysdeps/x86_64/x32 -I ~/src/glibc/include -I ~/src/glibc/build/ -I ~/src/glibc


.PHONY : clean
clean : 
	rm -f memchr_avx2 memchr_sse
