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
JIT=$3

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
    jstat -gcutil ${execId} 1000 > ${OUTPUT}/jstat.txt &

	if [ $JIT -eq 1 ]
	then
		jstat -printcompilation ${execId} 1000 > ${OUTPUT}/jit_method.txt &
		jstat -compiler ${execId} 1000 > ${OUTPUT}/jit.txt &
	fi
    i=$((i + 1))

done
