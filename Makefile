CC = gcc
CFLAGS = -Wall
CFLAGS += -march=native
#CFLAGS += -march=icelake-client
#CFLAGS += -Wno-unused-function -Wno-parentheses
AVX = -D__FUNC_CALL__="__memchr_avx2"
SSE = -D__FUNC_CALL__="__memchr_sse"

all : memchr_sse memchr_avx2 memchr_simple memchr_glibc
.PHONY : all

memchr_sse: memchr_sse.o memchr_main.c
	$(CC) $(CFLAGS) -o memchr_sse memchr_sse.o memchr_main.c $(SSE)

memchr_sse.o : memchr_sse.S
	$(CC) $(CFLAGS) -c memchr_sse.S -I ~/src/glibc/sysdeps/x86_64 -I ~/src/glibc/sysdeps/x86_64/x32 -I ~/src/glibc/include -I ~/src/glibc/build/ -I ~/src/glibc

memchr_avx2: memchr_avx2.o memchr_main.c
	$(CC) $(CFLAGS) -o memchr_avx2 memchr-avx2.o memchr_main.c $(AVX) 

memchr_avx2.o : memchr_avx2.S
	$(CC) $(CFLAGS) -c memchr_avx2.S -I ~/src/glibc/sysdeps/x86_64 -I ~/src/glibc/sysdeps/x86_64/x32 -I ~/src/glibc/include -I ~/src/glibc/build/ -I ~/src/glibc

memchr_glibc : memchr_glibc_O memchr_glibc_O2 memchr_glibc_O3

memchr_simple : memchr_simple_O memchr_simple_O2 memchr_simple_O3

memchr_glibc.o : memchr_glibc.c
	$(CC) $(CFLAGS) -c memchr_glibc.c

memchr_glibc_O.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O -c -o memchr_glibc_O.o

memchr_glibc_O2.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O2 -c -o memchr_glibc_O2.o

memchr_glibc_O3.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O3 -c -o memchr_glibc_O3.o

memchr_simple.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c

memchr_simple_O.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O -c -o memchr_simple_O.o

memchr_simple_O2.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O2 -c -o memchr_simple_O2.o

memchr_simple_O3.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O3 -c -o memchr_simple_O3.o

.PHONY : clean
clean : 
	rm -f memchr_avx2 memchr_sse *.o
