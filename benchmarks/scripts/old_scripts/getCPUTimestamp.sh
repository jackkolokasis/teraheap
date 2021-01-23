#!/usr/bin/env bash

LOG_START=start.txt
LOG_END=stop.txt

getCPU() {
    local output=$1
    cat /proc/stat | grep "cpu " > ${output}
}

while (true)
do
    getCPU ${LOG_START}
    getCPU ${LOG_END}

    ./cpuUtil.sh ${LOG_START} ${LOG_END}

    sleep 0.1
done
