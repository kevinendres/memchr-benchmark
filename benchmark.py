import subprocess
import shlex
import json
import datetime

#### Prepare input (set variables, ingest C code, etc)
#### run code repeatedly (store data in list/file/variable)
#### process data

benchmark_c_file = "./memchr_benchmark.c"
current_o_file = ''
#current_o_file = "/Users/kevin/coding/summer_research_2020/benchmarking/memchr_benchmarking/memchr_benchmark/memchr-avx2.o"

#values passed into C
BUFFER_SIZE = 1E9               # 1 GD even
PWR_TWO_BUFFER = 1073741824     # 2^30
SEARCH_STR_STATIC = 0x11
MEM_FILLER_STATIC = 0x00

def assemble(c_file=benchmark_c_file, buffer_size=BUFFER_SIZE, search_str=SEARCH_STR_STATIC,
    mem_filler=MEM_FILLER_STATIC, additional_options='', output_name="memchr_bench.S"):
    command_line = "gcc -S {C_File} -DBUFFER_SIZE={Buffer_Size} -DMEM_FILLER={Mem_Filler} -DSEARCH_STR={Search_Str} -o {Output_Name}".format(
        C_File = c_file,
        Buffer_Size = buffer_size,
        Mem_Filler = mem_filler,
        Search_Str = search_str,
        Output_Name = output_name)
    args = shlex.split(command_line)
    subprocess.run(args)

def compile(c_file=benchmark_c_file, o_file=current_o_file, buffer_size=BUFFER_SIZE, search_str=SEARCH_STR_STATIC,
    mem_filler=MEM_FILLER_STATIC, additional_options='', output_name="memchr_bench", assembly=False):
    '''compile based on specified parameters. Default executable is 'memchr_bench'. '''
    command_line = "gcc {O_File} {C_File} -DBUFFER_SIZE={Buffer_Size} -DMEM_FILLER={Mem_Filler} -DSEARCH_STR={Search_Str} {Options} -o {Output_Name}".format(
        C_File = c_file,
        O_File = o_file,
        Buffer_Size = buffer_size,
        Mem_Filler = mem_filler,
        Search_Str = search_str,
        Options = additional_options,
        Output_Name = output_name)
    args = shlex.split(command_line)
    if (assembly == True):
        assemble(c_file, buffer_size, search_str, mem_filler, additional_options, output)
    subprocess.run(args)

compile()
c_executable = "./memchr_bench"

# Experiment loop variables
times = list()
i = 0
repetitions = 1000

# Experiment loop
print("beginning loop")
while (i < repetitions):
    output = subprocess.run(c_executable, capture_output=True)
    output = int(output.stdout.decode("ascii"))
    times.append(output)
    #print(i)
    i += 1
    #print("loop " + str(i) + " is complete")

# output processing
experiment_date = datetime.datetime.now().replace(microsecond=0).isoformat()
results_sum = sum(times)
mean = results_sum / repetitions
results_json = {"date": experiment_date, "mean time": mean, "experiment repetitions": repetitions, "experiment run times": times, "memchr loops per experiment": 0, "loop run times": 0}

with open('results.json', 'a') as outfile:
    json.dump(results_json, outfile)

# data processing
