#!/usr/bin/env bash

PROFILER_PATH=/home1/public/kolokasis/async-profiler-1.5
OUTPUT_FILE=$1

while [ -z "${JVM_PROCESS}" ]
do
    JVM_PROCESS=$(jps | grep "CoarseGrainedExecutorBackend" | cut -d' ' -f1 | head -n 1)
done

${PROFILER_PATH}/profiler.sh -d 10000 -o svg ${JVM_PROCESS} > ${OUTPUT_FILE} 

# for executor in ${JVM_PROCESS}
# do
#     echo "Export flemeGraph..........."
#     ${PROFILER_PATH}/profiler.sh 
#     exit
# done
