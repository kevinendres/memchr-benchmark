import subprocess
import statistics
import datetime
import csv

avx2_exe = "memchr_avx2"
sse_exe = "memchr_sse"
glibc_exe = "memchr_glibc"
simple_exe = "memchr_simple"
executables = [avx2_exe, sse_exe, glibc_exe, simple_exe]
optimizations = ["_O", "_O2", "_O3"]
for exe in [glibc_exe, simple_exe]:
    for i in optimizations:
        executables.append(exe+i)

# Experiment loop variables
repetitions = 2
execution_times = list()

# Experiment loop
for exe in executables:
    i = 0
    path_exe = "./" + exe
    times = list()
    print("beginning loop")
    while (i < repetitions):
        sub_proc = subprocess.Popen(path_exe, stdout=subprocess.PIPE)
        output, err = sub_proc.communicate()
        output = int(output.decode("ascii"))
        times.append(output)
        print(i)
        i += 1
    execution_times.append(times)

# output processing
with open('results.csv', 'a', newline='') as csv_file:
    fields = ["Executable", "Min", "Max", "Mean", "Median", "Std. Deviation", "Time Stamp"]
    csv_writer = csv.DictWriter(csv_file, fieldnames=fields)
    csv_writer.writeheader()
    for i, times in enumerate(execution_times):
        min_time = min(times)
        max_time = max(times)
        mean_time = statistics.mean(times)
        median_time = statistics.median(times)
        std_dev = statistics.stdev(times) 
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        exe = executables[i]
        csv_writer.writerow({ "Executable" : exe, "Min" : min_time, "Max" : max_time, "Mean" : mean_time,\
                "Median" : median_time, "Std. Deviation" : std_dev, "Time Stamp" : time_stamp })
