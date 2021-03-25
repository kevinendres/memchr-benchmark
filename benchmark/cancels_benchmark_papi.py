import subprocess
import statistics
import datetime
import csv

avx2_exe = "/home/kevin/coding/memchr_benchmark/parallel_bins/cancels/memchr_avx2_papi_full"

# Experiment variables
repetitions = 750
thread_counts = range(1,21)
buffer_size = "4000000000"

execution_times = list()
configurations = list()

# Experiment loop
for thread_count in thread_counts:
    i = 0
    while (i < repetitions):
        # Execution loop that runs repetitions times for each configuration
        sub_proc = subprocess.Popen([avx2_exe, "-t", str(thread_count), "-d", buffer_size], stdout=subprocess.PIPE)
        output, err = sub_proc.communicate()
        execution_times.append(output.decode("ascii"))
        configurations.append(thread_count)
        i += 1

# output writing
j = 0
with open('papi_cancels_results.csv', 'a', newline='') as csv_file:
    csv_file.write("Executable,Threads,Buffer Size,Papi execution time,Time Stamp\n")
    for time in execution_times:
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        threads = configurations[j]
        j += 1
        csv_file.write("memchr_avx2," + str(threads) + ",4GB," + time + ',' + str(time_stamp) + '\n')
