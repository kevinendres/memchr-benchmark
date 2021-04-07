project('memchr_benchmark', 'c',
        version : '0.3',
        license : 'Unlicense',
        default_options : 'warning_level=3')

project_sources = []

#libs
pthreads = dependency('threads')
papi = dependency('papi')
project_deps = [pthreads, papi]

#compiler flages
add_global_arguments('-march=native', '-Ofast', language : 'c')
avx512_flag =  ['-mavx512f']
unroll_flags = ['-funroll-loops', '-fvariable-expansion-in-unroller']

#subdirs
subdir('src')

memchr_bin = executable('memchr_test',
                        project_sources,
                        dependencies : project_deps)
