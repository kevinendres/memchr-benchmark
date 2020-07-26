import subprocess
import statistics
import datetime
import csv

avx2_exe = "simd_overhead_avx2"
sse_exe = "simd_overhead_sse"
executables = [avx2_exe, sse_exe]

# Experiment loop variables
repetitions = 50 
execution_times = list()

# Experiment loop
for exe in executables:
    i = 0
    path_exe = "./" + exe
    times = list()
    print("beginning loop")
    while (i < repetitions):
        output = subprocess.run(path_exe, capture_output=True)
        output = output.stdout.decode("ascii").split('\n')
        output.pop()
        for j, call in enumerate(output):
            output[j] = int(call)
        times.append(output)
        print(i)
        i += 1
    execution_times.append(times)

# output processing
with open('simd_results.csv', 'a', newline='') as csv_file:
    fields = ["Executable", "Min", "Max", "Mean", "Median", "Std. Deviation", "Time Stamp"]
    csv_writer = csv.DictWriter(csv_file, fieldnames=fields)
    csv_writer.writeheader()
    #initial call
    for i, time_sets in enumerate(execution_times):
        firsts = list()
        for array in time_sets:
            firsts.append(array[0])
        exe = executables[i] + " first call"
        min_time = min(firsts)
        max_time = max(firsts)
        mean_time = statistics.mean(firsts)
        median_time = statistics.median(firsts)
        std_dev = statistics.stdev(firsts) 
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        csv_writer.writerow({ "Executable" : exe, "Min" : min_time, "Max" : max_time, "Mean" : mean_time,\
                "Median" : median_time, "Std. Deviation" : std_dev, "Time Stamp" : time_stamp })
    #repeated calls
    for i, time_sets in enumerate(execution_times):
        times = list()
        for array in time_sets:
            for nested_value in array[1:]:
                times.append(nested_value)
        exe = executables[i] + " repetead calls"
        min_time = min(times)
        max_time = max(times)
        mean_time = statistics.mean(times)
        median_time = statistics.median(times)
        std_dev = statistics.stdev(times) 
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        csv_writer.writerow({ "Executable" : exe, "Min" : min_time, "Max" : max_time, "Mean" : mean_time,\
                "Median" : median_time, "Std. Deviation" : std_dev, "Time Stamp" : time_stamp })
