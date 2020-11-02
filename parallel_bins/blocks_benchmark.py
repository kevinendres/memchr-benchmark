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
thread_counts = [2, 4, 5, 6, 7, 8, 10, 12, 15, 18, 19, 20, 21, 22, 25, 27, 28, 30, 34, 37, 40, 41]
buffer_sizes = [19000000, 25000000, 50000000, 100000000, 256000000, 500000000, 1000000007, 2000000011, 4000000007, 8000000011]

execution_times = list()
configurations = list()

# Experiment loop
for exe in executables:
    path_exe = "/home/kevin/coding/memchr_benchmark/parallel_bins/blocks/" + exe
    print("beginning loop")
    for thread_count in thread_counts:
        print("thread_count is " + str(thread_count))
        for buffer_size in buffer_sizes:
            print("buffer_size is " + str(buffer_size))
            block_size = 1000
            increment = 1000
            while (block_size < buffer_size):
                # Block size increment loop
                times = list()
                i = 0
                while (i < repetitions):
                    # Execution loop that runs repetitions times for each configuration
                    sub_proc = subprocess.Popen([path_exe, "-t", str(thread_count), "-d", str(buffer_size), "-b", str(block_size)], stdout=subprocess.PIPE)
                    output, err = sub_proc.communicate()
                    output = int(output.decode("ascii"))
                    times.append(output)
                    print(exe + " threads: " + str(thread_count) + " buffer size: " + str(buffer_size) + " block size: " + str(block_size))
                    i += 1
                # record times and configuration
                execution_times.append(times)
                configurations.append(exe)
                configurations.append(thread_count)
                configurations.append(buffer_size)
                configurations.append(block_size)

                # increase block size
                if (block_size >= 1000000000):
                    increment = 512000000
                elif (block_size >= 128000000):
                    increment = 128000000
                elif (block_size >= 16000000):
                    increment = 16000000
                elif (block_size >= 2000000):
                    increment = 2000000
                elif (block_size >= 512000):
                    increment = 512000
                elif (block_size >= 64000):
                    increment = 64000
                elif (block_size >= 4000):
                    increment = 4000
                block_size += increment

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
with open('blocks_results.csv', 'a', newline='') as csv_file:
    fields = ["Executable", "Threads", "Buffer Size", "Block Size", "GB/s", "Min", "Max", "Mean", "Median", "Std. Deviation", "Time Stamp"]
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
        block = configurations[j]
        j += 1
        gb_s = buf / mean_time
        csv_writer.writerow({ "Executable" : exe, "Threads" : threads, "Buffer Size" : buf, \
            "Block Size" : block, "GB/s" : gb_s, "Min" : min_time, "Max" : max_time, \
            "Mean" : mean_time, "Median" : median_time, "Std. Deviation" : std_dev, "Time Stamp" : time_stamp, })
