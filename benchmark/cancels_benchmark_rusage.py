import subprocess
import statistics
import datetime
import csv

avx2_exe = "/home/kevin/coding/memchr_benchmark/parallel_bins/cancels/memchr_avx2_papi_rusage"

# Experiment variables
repetitions = 1000
thread_counts = range(1,41)
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
with open('avx2_rusage_cancels_results.csv', 'a', newline='') as csv_file:
    csv_file.write("Time Stamp,Executable,Threads,Buffer Size,Papi execution time,Userspace time,Kernel time,Max Resident Set Size,Page reclaims,\
Page faults,Papi Virtual Cycles,Block out ops,Voluntary Context Switches,Involuntary Context Switches,Spawn Delay,Work Time,Join Delay\n")
    for time in execution_times:
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        threads = configurations[j]
        j += 1
        csv_file.write(str(time_stamp) + ",memchr_avx2," + str(threads) + ",4GB," + time +  '\n')