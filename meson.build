project('memchr_benchmark', 'c',
        version : '0.3',
        license : 'Unlicense',
        default_options : 'warning_level=3')

project_sources = []

#libs
pthreads = dependency('threads')
papi = dependency('papi')
libfyaml = dependency('libfyaml')
project_deps = [pthreads, papi, libfyaml]

#compiler flages
add_global_arguments('-march=native', language : 'c')
avx512_flag =  ['-mavx512f']
unroll_flags = ['-funroll-loops', '-fvariable-expansion-in-unroller']

#subdirs
subdir('src')

memchr_bin = executable('memchr_benchmarking',
                        project_sources,
                        dependencies : project_deps)
