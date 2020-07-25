import subprocess
import json
import statistics
import datetime
import csv

avx2_exe = "memchr_avx2"
sse_exe = "memchr_sse"
glibc_exe = "memchr_glibc"
simple_exe = "memchr_simple"
executables = [avx2_exe, sse_exe, glibc_exe, simple_exe]
optimizations = list("_O", "_O2", "_O3")
for exe in (glibc_exe, simple_exe):
    for i in optimizations:
        executables.append(exe+i)

# Experiment loop variables
repetitions = 3
execution_times = list()

# Experiment loop
for exe in executables:
    i = 0
    path_exe = "./" + exe
    times = list()
    print("beginning loop")
    while (i < repetitions):
        output = subprocess.run(path_exe, capture_output=True)
        output = int(output.stdout.decode("ascii"))
        times.append(output)
        print(i)
        i += 1
    execution_times.append(times)

# output processing
for i, times in enumerate(execution_times):
    min_time = min(times)
    max_time = max(times)
    mean_time = statistics.mean(times)
    median_time = statistics.median(times)
    std_dev = statistics.stdev(times) 
    time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
    exe = executables[i]
    
with open('results.csv', 'a', newline='') as csv_file:
    fields = ["Executable", "Min", "Max", "Mean", "Median", "Std. Deviation"]
    csv_writer = csv.DictWriter(csv_file, fieldnames=fields)
    csv_writer.writerow({ "Executable" : exe, "min" : min_time, "max" : max_time, "mean" : mean_time,\
            "median" : median_time, "standard deviation" : std_dev, "time stamp" : time_stamp })


# data processing
