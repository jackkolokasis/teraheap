#!/usr/bin/env bash

###################################################
#
# file: 1.mlExperiments
#
# @Author:   Iacovos G. Kolokasis
# @Version:  20-02-2019
# @email:    kolokasis@ics.forth.gr
#
# Run Experiments
#----------+--------+-------+
# HostName |  SSD   | NVMe  |
#----------+--------+-------+
# sith0    |  sdb   |       |
# sith1    |  sdc   |nvme0n1|
# sith2    |  sdc   |       |
# sith3    |  sdc   |nvme0n1|
# sith4    |        |nvme0n1|
# sith5    |        |nvme0n1|
#-------------------+-------+
#
# Before run the test make sure that devices are
# correct mounted to each device name
###################################################

export PYSPARK_PYTHON=python3 

DEVICES=( "SSD" "NVMe" )
PARTITIONS=( 500 )
#BENCHMARKS=( "ConnectedComponent" "LabelPropagation" "PageRank" "ShortestPaths"\
#             "SVDPlusPlus" "TriangleCount" )
MEM_PER_EXECUTOR=1
EXECUTORS=6
BENCHMARKS=( "PageRank" )

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -n  Number of Runs"
    echo "      -t  Type of Storage Device (e.g SSD, NVMe, NVM)"
    echo "      -c  Number of cores"
    echo "      -d  Dataset size (e.g 10GB, 20GB)"
    echo "      -o  Output Path"
    echo "      -i  SparkBench ID (e.g sith0, sith1)"
    echo "      -k  Kill iostat and jstat processes"
    echo "      -h  Show usage"
    echo

    exit 1
}

##
# Description: 
#   Start Spark
##
startSpark() {
    /opt/spark/spark-2.3.0-kolokasis/sbin/start-all.sh
}

##
# Description: 
#   Stop Spark
##
stopSpark() {
    /opt/spark/spark-2.3.0-kolokasis/sbin/stop-all.sh
}

##
# Description: 
#   Kill running background processes (iostat, jstat)
#
# Arguments:
#   $1 - Restart Spark
##
killProcess() {
    local iostatPID=$(pgrep iostat)
    local myjstatPID=$(pgrep myjstat)

    # Kill all iostat process
    for iostat_pid in ${iostatPID}
    do
        kill -KILL ${iostat_pid}
    done

    # Kill all jstat process
    for jstat_pid in ${myjstatPID}
    do
        kill -KILL ${jstat_pid}
    done

    if [[ $1 == "restart" ]]
    then
        stopSpark
        startSpark
    fi
}

##
# Description: 
#   Check if nvme devices are mounted correctly
#
# Arguments:
#   $1 - Device name
#
# Returns:
#   Exit on failure
##
checkDevices() {
    local check=$(lsblk -o NAME | grep $1)

    if [[ ${check} == "" ]]
    then
        echo "ERROR: $1 device does not exist"
        exit 1
    fi

    mountPoint=$(lsblk -o NAME,MOUNTPOINT | grep $1 | cut -d' ' -f2)

    if [[ ${mountPoint} == "" ]]
    then
        echo "Mount $1 device"
        sudo mount /dev/nvme0n1 /nvme
    fi
}

##
# Description: 
#   Export Log file for each experiment 
#
# Arguments:
#   $1 - Output Path
#   $2 - Storage Type
#   $3 - Input dataset size
#   $4 - Number of partitions
#
# Returns:
#    1 on success or 0 otherwise
##
exportLogFile() {
    local outFile=$1/README                         # Ouput file name
    local hostName=$(hostname | cut -d'.' -f1)      # Host name
    local deviceName=""                             # Storage device name

    echo "Host,${hostName}" > ${outFile}
    echo "StorageType,$2" >> ${outFile}

    if [ $2 == "SSD" ]
    then
        case ${hostName} in
            "sith0")
                deviceName="sdb"
                ;;
            *)
                deviceName="sdc"
                ;;
        esac
    else
        deviceName="nvme0n1"
    fi

    echo "DeviceName,${deviceName}" >> ${outFile}
    echo "DataSize,$3" >> ${outFile}
    echo "Partitions,$4" >> ${outFile}
    echo "MEM_PER_EXECUTOR,${MEM_PER_EXECUTOR}" >> ${outFile}
    echo "ΕΧΕCUTORS,${EXECUTORS}" >> ${outFile}
    echo "Date,$(date +%Y-%m-%d)" >> ${outFile}
}

##
# Description: 
#   Remove executors log files
##
cleanWorkDirs() {

    cd /opt/spark/spark-2.3.0-kolokasis/work/

    for f in $(ls)
    do
        if [[ $f == "app-"* ]]
        then
            rm -rf ${f}
        fi
    done

    cd -
}

##
# Description: 
#   Set workload configuration parameters
#
# Arguments:
#   $1 - Workload directory
#   $2 - Number of partitions
#
# Returns:
#    Exit on failure
##
        
setConf() {
    local workloadDir=$1                    # Workload directory
    local numPart=$2                        # Number of partitions
 
    # Set number of partitions as parameter
    sed -i 's/NUM_OF_PARTITIONS=\([a-z0-9]*\)/NUM_OF_PARTITIONS='${numPart}'/' ${workloadDir}/conf/env.sh

    # Check configuration parameters correctness
    check=$(cat ${workloadDir}/conf/env.sh | grep "NUM_OF_PARTITIONS" | cut -d'=' -f2)

    if [[ ${check} != ${numPart} ]]
    then
        echo "FAILED: Set ${workload} Configuration file parameters"
        exit 1
    fi
}

##
# Description: 
#   Export LoadTime and SaveTime from results output
#
# Arguments:
#   $1 - Benchmark Directory
#   $2 - File to parse
#
##
parseResult() {
    local benchDir=$1
    local fileToParse=$2

    cat ${fileToParse} | grep "LoadTime" > ${benchDir}/total_time.txt
    cat ${fileToParse} | grep "ParseTime" >> ${benchDir}/total_time.txt
    cat ${fileToParse} | grep "ComputationTime" >> ${benchDir}/total_time.txt
    cat ${fileToParse} | grep "SaveTime" >> ${benchDir}/total_time.txt

    rm ${fileToParse}
}

##
# Description: 
#   Console Message
#
# Arguments:
#   $1 - Device Name
#   $2 - Workload Name
#
##
printStartMsg() {
    echo
    echo "====================================================================="
    echo 
    echo "GRAPH EXPERIMENTS"
    echo
    echo "      DEVICE   : $1"
    echo "      WORKLOAD : $2"
    echo
    echo -n "       Number of Partitions: "
    
}

##
# Description: 
#   Console Message
#
# Arguments:
#   $1 - End Time
#   $2 - Start Time
#
##
printEndMsg() {
    ELAPSEDTIME=$(($2 - $1))
    FORMATED="$(($ELAPSEDTIME / 3600))h:$(($ELAPSEDTIME % 3600 / 60))m:$(($ELAPSEDTIME % 60))s"  
    echo
    echo
    echo "    Benchmark Time Elapsed: $FORMATED"
    echo
    echo "====================================================================="
    echo
}

# Check for the input arguments
while getopts ":n:t:c:o:i:d:m:kh" opt
do
    case "${opt}" in
        n)
            ITER=${OPTARG}
            ;;
        t)
            TYPE=${OPTARG}
            ;;
        c)
            CORE=${OPTARG}
            ;;
        o)
            OUTPUT_PATH=${OPTARG}
            ;;
        i)
            ID=${OPTARG}
            ;;
        d)
            DATA_SIZE=${OPTARG}
            ;;
        k)
            killProcess "restart"
            exit 1
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

# Check if NVMe device is mounted correctly
#checkDevices nvme0n1

mkdir -p ${OUTPUT_PATH}

OUT="${OUTPUT_PATH}"

for benchmark in "${BENCHMARKS[@]}"
do
    printStartMsg ${TYPE} ${benchmark}

    STARTTIME=$(date +%s)
    for partitions in "${PARTITIONS[@]}"
    do
        mkdir -p ${OUT}/${TYPE}/${CORE}/${partitions}
        BENCH_DIR="${OUT}/${TYPE}/${CORE}/${partitions}"

        # Set the configuration parameters for each benchmark
        setConf "../spark-bench-legacy_${ID}/${benchmark}" ${partitions}

        echo -n "${partitions} "
        
        for ((i=0; i<${ITER}; i++))
        do
            # Create a directory for each run
            mkdir -p ${BENCH_DIR}/${benchmark}/run_${i}
            RUN_DIR="${BENCH_DIR}/${benchmark}/run_${i}"

            exportLogFile ${RUN_DIR} ${TYPE} ${DATA_SIZE} ${partitions}

            # JSTAT Statistics [background process]
            # ./myjstat.sh ${RUN_DIR}/gcTime.txt &

            # Drop caches
            sudo sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

            # IOSTAT Statistics [background process]
            iostat -xm 1 > ${RUN_DIR}/iostat.txt & 

            # Run benchmark and save output to tmp_out.txt
            ../spark-bench-legacy_${ID}/${benchmark}/bin/run.sh \
                > ${RUN_DIR}/tmp_out.txt

            # Kill all background processes
            killProcess
            
            # Parse the results of tmp_out.txt
            parseResult ${RUN_DIR} ${RUN_DIR}/tmp_out.txt

            # Save the total duration of the benchmark execution
            tail -n 1 ../spark-bench-legacy_${ID}/num/bench-report.dat \
                >> ${RUN_DIR}/total_time.txt

            # Clean executors log files
            cleanWorkDirs

            # Collect Spark Metrics Value
            /opt/spark/spark-2.3.0-kolokasis/bin/spark-submit \
                sparkMeasureReport.py -o ${RUN_DIR}/sparkMetrics.csv

            # Remove sparkMeasure file
            rm -rf taskMetrics.serialized
        done
    done

    ENDTIME=$(date +%s)
    printEndMsg ${STARTTIME} ${ENDTIME}
done
