project('memchr_benchmark', 'c')

pthreads = dependency('threads')

#compiler flages
add_global_arguments('-Wall', '-march=native')
avx512_flag =  ['-mavx512f']
unroll_flags = ['-funroll-loops', '-fvariable-expansion-in-unroller']

inc = include_directories('include')

subdir('include')
subdir('src')

