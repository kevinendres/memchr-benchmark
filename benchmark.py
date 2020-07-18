import subprocess
import json
import statistics
import datetime

avx2_exe = "memchr_avx2"
sse_exe = "memchr_sse"
executables = [avx2_exe, sse_exe]

# Experiment loop variables
repetitions = 1000
execution_times = list()

# Experiment loop
for exe in executables:
    i = 0
    times = list()
    print("beginning loop")
    while (i < repetitions):
        output = subprocess.run(exe, capture_output=True)
        output = int(output.stdout.decode("ascii"))
        times.append(output)
        print(i)
        i += 1
    execution_times.append(times)

# output processing
json_results = dict()
for i, times in enumerate(execution_times):
    min_time = min(times)
    max_time = max(times)
    mean_time = statistics.mean(times)
    median_time = statistics.median(times)
    std_dev = statistics.stdev(times) 
    time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
    json_results[executables[i]] = { "min" : min_time, "max" : max_time, "mean" : mean_time,\
            "median" : median_time, "standard deviation" : std_dev, "time stamp" : time_stamp }
    
with open('results.json', 'a') as outfile:
    json.dump(json_results, outfile)

# data processing
