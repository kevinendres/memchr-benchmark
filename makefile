CC = gcc
CFLAGS = -Wall -O3
CFLAGS += -march=native
#CFLAGS += -march=icelake-client
#CFLAGS += -Wno-unused-function -Wno-parentheses
AVX = -D__FUNC_CALL__="__memchr_avx2"
SSE = -D__FUNC_CALL__="__memchr"

all : memchr_sse memchr_avx2
.PHONY : all

memchr_sse: memchr.o memchr_main.c
	$(CC) $(CFLAGS) -o memchr_sse memchr.o memchr_main.c $(SSE)
	
memchr_avx2: memchr-avx2.o memchr_main.c
	$(CC) $(CFLAGS) -o memchr_avx2 memchr-avx2.o memchr_main.c $(AVX) 

.PHONY : clean
clean : 
	rm -f memchr_avx2 memchr_sse
