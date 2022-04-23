#!/usr/bin/env bash

###################################################
#
# file: perf.sh
#
# @Author:  Iacovos G. Kolokasis
# @Version: 26-03-2021
# @email:   kolokasis@ics.forth.gr
#
# @brief    This script uses the perf to monitor
# tha total:
#	- cache references
#	- cache misses 
#	- pagefaults
#
###################################################

# Output file name
OUTPUT=$1        
NUM_OF_EXECUTORS=$2        

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

for execId in ${processId}
do
	perf stat -o ${OUTPUT} -e cache-references,cache-misses,page-faults,major-faults,minor-faults,dTLB-load-misses,dTLB-store-misses -p ${execId} &

	#perf stat -o ${OUTPUT} -e cache-references,cache-misses \
	#	-e dTLB-load-misses,dTLB-store-misses \
	#	-e L1-dcache-load-misses,L1-dcache-loads \
	#	-e LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores -p ${execId} &
	
	#perf stat -o ${OUTPUT} -e cpu-cycles,instructions -p ${execId} &
done

exit
