#!/usr/bin/env bash

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

#Get the proccess id from the running
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
    i=$((i + 1))
done

# Driver
driverId=$(jps | grep "SparkSubmit" |\
    awk '{split($0, array, " "); print array[1]}')

jstat -gcutil ${driverId} 1000 > ${OUTPUT}_${i}.txt &

# Concantenate in an array all the process of spark 
# (executors + master)
processId+=(${driverId})

# Convert bash array into a delimited string
str=$( IFS=','; echo "${processId[*]}" )

# Perf events trace
#perf stat -e cache-references,cache-misses,major-faults,minor-faults \
#          -p ${listIds}

# Perf events trace
perf stat -e task-clock,cycles,instructions,branches,branch-misses \
          -e stalled-cycles-frontend,stalled-cycles-backend \
          -e cache-references,cache-misses \
          -e LLC-loads,LLC-load-misses,LLC-stores,LLC-store-misses \
          -e L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses \
          -e mem-loads,mem-stores,page-faults \
          -p ${str} > ${PERF_OUTPUT} &

exit
