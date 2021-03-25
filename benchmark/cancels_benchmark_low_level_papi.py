import subprocess
import statistics
import datetime
import csv

path = "/home/kevin/coding/memchr_benchmark/parallel_bins/cancels/"
exes = list()
#for i in range(1, 7):
#    exes.append("memchr_avx2_hacked_set" + str(i))
exes.append("memchr_avx2_hacked_set1") #to makeup for missing execution set 1/23/2021
events = ["PAPI_L1_DCM","PAPI_L1_ICM","PAPI_L2_DCM","PAPI_L2_ICM","PAPI_L1_TCM","PAPI_L2_TCM","PAPI_L3_TCM","PAPI_CA_SNP","PAPI_CA_SHR","PAPI_CA_CLN","PAPI_CA_ITV","PAPI_L3_LDM","PAPI_TLB_DM","PAPI_TLB_IM","PAPI_L1_LDM","PAPI_L1_STM","PAPI_L2_LDM","PAPI_L2_STM","PAPI_PRF_DM","PAPI_MEM_WCY","PAPI_STL_ICY","PAPI_FUL_ICY","PAPI_STL_CCY","PAPI_FUL_CCY","PAPI_BR_UCN","PAPI_BR_CN","PAPI_BR_TKN","PAPI_BR_NTK","PAPI_BR_MSP","PAPI_BR_PRC","PAPI_TOT_INS","PAPI_LD_INS","PAPI_SR_INS","PAPI_BR_INS","PAPI_RES_STL","PAPI_TOT_CYC","PAPI_LST_INS","PAPI_L2_DCA","PAPI_L3_DCA","PAPI_L2_DCR","PAPI_L3_DCR","PAPI_L2_DCW","PAPI_L3_DCW","PAPI_L2_ICH","PAPI_L2_ICA","PAPI_L3_ICA","PAPI_L2_ICR","PAPI_L3_ICR","PAPI_L2_TCA","PAPI_L3_TCA","PAPI_L2_TCR","PAPI_L3_TCR","PAPI_L2_TCW","PAPI_L3_TCW","PAPI_SP_OPS","PAPI_DP_OPS","PAPI_VEC_SP","PAPI_VEC_DP","PAPI_REF_CYC"]

# Experiment variables
repetitions = 50
thread_counts = range(1, 21)
buffer_size = "4000000000"

# Experiment loop
for exe in exes:
    print("beginning " + str(exe))
    execution_times = list()
    configurations = list()
    for thread_count in thread_counts:
        i = 0
        while (i < repetitions):
            # Execution loop that runs repetitions times for each configuration
            sub_proc = subprocess.Popen([path + exe, "-t", str(thread_count), "-d", buffer_size], stdout=subprocess.PIPE)
            output, err = sub_proc.communicate()
            execution_times.append(output.decode("ascii"))
            configurations.append(thread_count)
            i += 1

    # output writing
    j = 0
    with open(exe + '.csv', 'a', newline='') as csv_file:
        time_stamp = datetime.datetime.now().replace(microsecond=0).isoformat()
        csv_file.write("Thread id,Process id,Main thread execution time,Spawn Delay,Total Work Time,Post Warmup Work Time,Join Delay,")
        for event in events:
            csv_file.write(event + ',')
        csv_file.write(str(time_stamp) + '\n')
        for time in execution_times:
            threads = configurations[j]
            j += 1
            csv_file.write(time)
    print("next exe")
