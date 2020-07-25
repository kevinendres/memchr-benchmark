CC = gcc
LD = ld
CFLAGS = -Wall
CFLAGS += -march=native
#CFLAGS += -march=icelake-client
#CFLAGS += -Wno-unused-function -Wno-parentheses
INCLUDES = -I ~/src/glibc/sysdeps/x86_64 -I ~/src/glibc/sysdeps/x86_64/x32 -I ~/src/glibc/include -I ~/src/glibc/build/ -I ~/src/glibc
AVX = -D__FUNC_CALL__="__memchr_avx2"
SSE = -D__FUNC_CALL__="__memchr_sse"
GLIBC = -D__FUNC_CALL__="__memchr_glibc"
SIMP = -D__FUNC_CALL__="__memchr_simple"

all : memchr_sse memchr_avx2 memchr_glibc memchr_simple 
.PHONY : all

memchr_sse: memchr_sse.o memchr_main.c
	$(CC) $(CFLAGS) -o $@ $^ $(SSE)

memchr_sse.o : memchr_sse.S
	$(CC) $(CFLAGS) -c memchr_sse.S $(INCLUDES) 

memchr_avx2: memchr_avx2.o memchr_main.c
	$(CC) $(CFLAGS) -o $@ $^ $(AVX) 

memchr_avx2.o : memchr_avx2.S
	$(CC) $(CFLAGS) -c memchr_avx2.S $(INCLUDES) 

memchr_glibc : memchr_glibc.o memchr_glibc_O.o memchr_glibc_O2.o memchr_glibc_O3.o
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc.o -o memchr_glibc
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_O.o -o memchr_glibc_O
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_O2.o -o memchr_glibc_O2
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_O3.o -o memchr_glibc_O3

memchr_glibc.o : memchr_glibc.c
	$(CC) $(CFLAGS) -c memchr_glibc.c 

memchr_glibc_O.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O -c -o $@

memchr_glibc_O2.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O2 -c -o $@

memchr_glibc_O3.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O3 -c -o $@

memchr_simple : memchr_simple.o memchr_simple_O.o memchr_simple_O2.o memchr_simple_O3.o
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple.o -o memchr_simple
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple_O.o -o memchr_simple_O
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple_O2.o -o memchr_simple_O2
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple_O3.o -o memchr_simple_O3

memchr_simple.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -c

memchr_simple_O.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O -c -o $@

memchr_simple_O2.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O2 -c -o $@

memchr_simple_O3.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O3 -c -o $@

BINS = memchr_avx2 memchr_sse memchr_simple memchr_glibc memchr_glibc_* memchr_simple_*
.PHONY : clean
clean : 
	rm -f $(BINS) *.o
