main = files('main.c')
avx512 = files('memchrc_avx512.c')
avx2 = files('memchr_avx2.S')


avx2 = executable('avx2',
                  main,
                  avx2,
                  include_directories : inc,
                 dependencies : pthreads)
