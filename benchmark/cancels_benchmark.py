import subprocess
import statistics
import datetime
import csv

avx2_exe = "memchr_avx2"
sse_exe = "memchr_sse"
glibc_exe = "memchr_glibc"
simple_exe = "memchr_simple"
throughput_exe = "throughput"
executables = [avx2_exe, sse_exe, glibc_exe, simple_exe]
optimizations = ["_O", "_O2", "_Ofast", "_unroll_O2"]
for exe in [glibc_exe, simple_exe]:
    for i in optimizations:
        executables.append(exe + i)

# Experiment variables
repetitions = 10
thread_counts = range(1,37)
buffer_sizes = [1000000000, 2000000000, 4000000000, 6000000000]

execution_times = list()
configurations = list()

# Experiment loop
for exe in executables:
    path_exe = "/home/kevin/coding/memchr_benchmark/parallel_bins/cancels/" + exe
    print("beginning loop")
    for thread_count in thread_counts:
        print("thread_count is " + str(thread_count))
        for buffer_size in buffer_sizes:
            print("buffer_size is " + str(buffer_size))
            times = list()
            i = 0
            while (i < repetitions):
                # Execution loop that runs repetitions times for each configuration
                sub_proc = subprocess.Popen([path_exe, "-t", str(thread_count), "-d", str(buffer_size)], stdout=subprocess.PIPE)
                output, err = sub_proc.communicate()
                output = int(output.decode("ascii"))
                times.append(output)
                print(exe + " threads: " + str(thread_count) + " buffer size: " + str(buffer_size))
                i += 1
            # record times and configuration
            execution_times.append(times)
            configurations.append(exe)
            configurations.append(thread_count)
            configurations.append(buffer_size)

# output processing
baseline_i = executables.index("memchr_simple")
baseline = statistics.mean(execution_times[baseline_i])
means = list()
speedups = list()
for time_set in execution_times:
    means.append(statistics.mean(time_set))
for mean in means:
    speedups.append(baseline / mean)

# output writing
j = 0
with open('cancels_results.csv', 'a', newline='') as csv_file:
    fields = ["Executable", "Threads", "Buffer Size", "GB/s", "Min", "Max", "Mean", "Median", "Std. Deviation", "Time Stamp"]
    csv_writer = csv.DictWriter(csv_file, fieldnames=fields)
    csv_writer.writeheader()
    for i, times in enumerate(execution_times):
        min_time = min(times)
        max_time = max(times)
        mean_time = means[i]
        median_time = statistics.median(times)
        std_dev = statistics.stdev(times)
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        exe = configurations[j]
        j += 1
        threads = configurations[j]
        j += 1
        buf = configurations[j]
        j += 1
        gb_s = buf / mean_time
        csv_writer.writerow({ "Executable" : exe, "Threads" : threads, "Buffer Size" : buf, \
            "GB/s" : gb_s, "Min" : min_time, "Max" : max_time, "Mean" : mean_time, \
            "Median" : median_time, "Std. Deviation" : std_dev, "Time Stamp" : time_stamp, })
