#!/bin/bash

###################################################
#
# file: myjstat.sh
#
# @Author:  Iacovos G. Kolokasis
# @Version: 04-05-2018
# @email:   kolokasis@ics.forth.gr
#
# @brief    This script use jstat to monitor the
# Garbage Collection utilization from the JVM
# executor running in spark. Find out the proccess
# id of the executor from the jps and the execute
# the jstat.  All the informations are saved in an
# output file.
#
###################################################

# Output file name
OUTPUT=$1
NUM_OF_EXECUTORS=$2
PERF_OUTPUT=$3

# Get the proccess id from the running
processId=""
numOfExecutors=0

while [ ${numOfExecutors} -lt ${NUM_OF_EXECUTORS} ]
do
    # Calculate number of executors running
    numOfExecutors=$(jps |grep "CoarseGrainedExecutorBackend" |wc -l)
done

# Executors
processId=$(jps |\
    grep "CoarseGrainedExecutorBackend" |\
    awk '{split($0,array," "); print array[1]}')

# Counter
i=0

for execId in ${processId}
do
    echo ${execId}
    jstat -gcutil ${execId} 1000 > ${OUTPUT}_${i}.txt &
    #/home1/public/kolokasis/sparkPersistentMemory/benchmarks/profiler/async-profiler/profiler.sh \
    #    -d 30000 -i 1ms ${execId} > ${PERF_OUTPUT}_$i.txt &

    sar -BS 1 > ${OUTPUT}_${i}_sar.txt &

    perf stat -e cache-misses,page-faults -p ${execId} 2> ${PERF_OUTPUT} &
    i=$((i + 1))
done

exit
