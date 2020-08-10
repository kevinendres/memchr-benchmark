CC = gcc
LD = ld
CFLAGS = -Wall
CFLAGS += -march=native
#CFLAGS += -march=icelake-client
#CFLAGS += -Wno-unused-function -Wno-parentheses
#INCLUDES = -I ~/src/glibc/sysdeps/x86_64 -I ~/src/glibc/sysdeps/x86_64/x32 -I ~/src/glibc/include -I ~/src/glibc/build/ -I ~/src/glibc
ALT_FLAGS_1 = -c -std=gnu11 -fgnu89-inline -g -O2 -Wall -Wwrite-strings -Wundef -Werror -fmerge-all-constants -frounding-math -fno-stack-protector -Wstrict-prototypes -Wold-style-definition -fmath-errno -ftls-model=initial-exec
ALT_FLAGS_2 = -c -std=gnu11 -fgnu89-inline -g -O2 -Wall -Wwrite-strings -Wundef -Werror -fmerge-all-constants -frounding-math -fno-stack-protector -Wstrict-prototypes -Wold-style-definition -fmath-errno -fPIC -ftls-model=initial-exec
ALT_FLAGS_3 = -c -std=gnu11 -fgnu89-inline -march=x86-64 -mtune=generic -O2 -pipe -Wall -Wwrite-strings -Wundef -fmerge-all-constants -frounding-math -fstack-protector-strong -Wstrict-prototypes -Wold-style-definition -fmath-errno -fPIC -fcf-protection -ftls-model=initial-exec
UNROLL = -funroll-loops -fvariable-expansion-in-unroller 
UNROLL_ALL = -funroll-all-loops -fvariable-expansion-in-unroller
AVX = -D__FUNC_CALL__="__memchr_avx2"
SSE = -D__FUNC_CALL__="__memchr_sse"
GLIBC = -D__FUNC_CALL__="__memchr_glibc"
SIMP = -D__FUNC_CALL__="__memchr_simple"

all : memchr_sse memchr_avx2 memchr_glibc memchr_simple simdo_avx2 simdo_sse memchr_unrolls throughput
.PHONY : all

memchr_sse: memchr_sse.o memchr_main.c
	$(CC) $(CFLAGS) -o $@ $^ $(SSE)

memchr_sse.o : memchr_sse.S
	$(CC) $(CFLAGS) -c memchr_sse.S 

memchr_avx2: memchr_avx2.o memchr_main.c
	$(CC) $(CFLAGS) -o $@ $^ $(AVX) 

memchr_avx2.o : memchr_avx2.S
	$(CC) $(CFLAGS) -c memchr_avx2.S 

memchr_glibc : memchr_glibc.o memchr_glibc_O.o memchr_glibc_O2.o memchr_glibc_Ofast.o memchr_glibc_altopts_1.o memchr_glibc_altopts_2.o memchr_glibc_altopts_3.o
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc.o -o memchr_glibc
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_O.o -o memchr_glibc_O
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_O2.o -o memchr_glibc_O2
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_Ofast.o -o memchr_glibc_Ofast
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_altopts_1.o -o memchr_glibc_altopts_1
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_altopts_2.o -o memchr_glibc_altopts_2
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) memchr_glibc_altopts_3.o -o memchr_glibc_altopts_3

memchr_glibc.o : memchr_glibc.c
	$(CC) $(CFLAGS) -c memchr_glibc.c 

memchr_glibc_O.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O -c -o $@

memchr_glibc_O2.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -O2 -c -o $@

memchr_glibc_Ofast.o : memchr_glibc.c
	$(CC) $(CFLAGS) memchr_glibc.c -Ofast -c -o $@

memchr_glibc_altopts_1.o : memchr_glibc.c
	$(CC) $(CFLAGS) $(ALT_FLAGS_1) memchr_glibc.c -o $@

memchr_glibc_altopts_2.o : memchr_glibc.c
	$(CC) $(CFLAGS) $(ALT_FLAGS_2) memchr_glibc.c -o $@

memchr_glibc_altopts_3.o : memchr_glibc.c
	$(CC) $(ALT_FLAGS_3) memchr_glibc.c -o $@

memchr_simple : memchr_simple.o memchr_simple_O.o memchr_simple_O2.o memchr_simple_Ofast.o
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple.o -o memchr_simple
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple_O.o -o memchr_simple_O
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple_O2.o -o memchr_simple_O2
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) memchr_simple_Ofast.o -o memchr_simple_Ofast

memchr_simple.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -c

memchr_simple_O.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O -c -o $@

memchr_simple_O2.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -O2 -c -o $@

memchr_simple_Ofast.o : memchr_simple.c
	$(CC) $(CFLAGS) memchr_simple.c -Ofast -c -o $@

simdo_avx2 : memchr_avx2.o
	$(CC) $(CCFLAGS) $< simd_overhead.c $(AVX) -o simd_overhead_avx2

simdo_sse : memchr_sse.o
	$(CC) $(CCFLAGS) $< simd_overhead.c $(SSE) -o simd_overhead_sse

throughput : throughput.o memchr_main.c
	$(CC) $(CFLAGS) -o $@ $^ $(AVX) 

throughput.o : throughput_test.S
	$(CC) $(CFLAGS) -c -o $@ $< 

memchr_unrolls : simple_unroll_O2.o glibc_unroll_O2.o simple_unrollall_O2.o glibc_unrollall_O2.o 
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) simple_unroll_O2.o -o memchr_simple_unroll_O2
	$(CC) $(CFLAGS) memchr_main.c $(SIMP) simple_unrollall_O2.o -o memchr_simple_unrollall_O2
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) glibc_unroll_O2.o -o memchr_glibc_unroll_O2
	$(CC) $(CFLAGS) memchr_main.c $(GLIBC) glibc_unrollall_O2.o -o memchr_glibc_unrollall_O2

simple_unroll_O2.o : memchr_simple.c
	$(CC) $(CFLAGS) $(UNROLL) -O2 -c -o $@ $<

simple_unrollall_O2.o : memchr_simple.c
	$(CC) $(CFLAGS) $(UNROLL_ALL) -O2 -c -o $@ $<

glibc_unroll_O2.o : memchr_glibc.c
	$(CC) $(CFLAGS) $(UNROLL) -O2 -c -o $@ $<

glibc_unrollall_O2.o : memchr_glibc.c
	$(CC) $(CFLAGS) $(UNROLL_ALL) -O2 -c -o $@ $<

BINS = memchr_avx2 memchr_sse memchr_simple memchr_glibc memchr_glibc_* memchr_simple_* simd_overhead_avx2 simd_overhead_sse throughput
.PHONY : clean
clean : 
	rm -f $(BINS) *.o
