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
# +---------+--------+-------+
# |HostName |  SSD   | NVMe  |
# +---------+--------+-------+
# | sith0   |  sdb   |       |
# | sith1   |  sdc   |nvme0n1|
# | sith2   |  sdc   |       |
# | sith3   |  sdc   |nvme0n1|
# | sith4   |        |nvme0n1|
# | sith5   |        |nvme0n1|
# +------------------+-------+
#
# Before run the test make sure that devices are
# correct mounted to each device name
###################################################

export PYSPARK_PYTHON=python3 

DEVICES=( "Optane" )
PARTITIONS=( 100 )
#BENCHMARKS=("KMeans" "LinearRegression" "LogisticRegression" "MatrixFactorization" "SVM")
MEM_PER_EXECUTOR=200
EXECUTORS=1
LOG_START="start.txt"
LOG_END="stop.txt"

#BENCHMARKS=( "KMeans" "LinearRegression" "LogisticRegression" )
BENCHMARKS=( "KMeans")

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
    local cachestatPID=$(pgrep cachestat)
    local perfPID=$(pgrep perf)
 
    # Kill perf process
    if [ -z ${perfPID} ]
    then
        echo "No perf process found"
    else
        kill -SIGINT ${perfPID} 2>/dev/null
    fi

    # Kill all iostat process
    for iostat_pid in ${iostatPID}
    do
        kill -KILL ${iostat_pid} 2>/dev/null
        wait ${iostat_pid} 2>/dev/null
    done

    # Kill all jstat process
    for jstat_pid in ${myjstatPID}
    do
        kill -KILL ${jstat_pid}
    done
    
    # Kill all cachestat process
    for cachestat_pid in ${cachestatPID}
    do
        sudo kill -KILL ${cachestat_pid} 2>/dev/null
        sudo wait ${cachestat_pid} 2>/dev/null
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
    elif [ $2 == "NVMe" ]
    then
        deviceName="nvme0n1"
    elif [ $2 == "OPTANE" ]
    then
        deviceName="nvme1n1"
    else
        deviceName="memory"
    fi

    echo "DeviceName,${deviceName}" >> ${outFile}
    echo "DataSize,$3" >> ${outFile}
    echo "Partitions,$4" >> ${outFile}
    echo "MEM_PER_EXECUTOR,${MEM_PER_EXECUTOR}" >> ${outFile}
    echo "NUM_EXECUTORS,${EXECUTORS}" >> ${outFile}
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
#   Check if the input argument
#
# Arguments:
#   $1 - Serialize / Deserialize time
#   
# Return:
#   return 0, if is empty otherwise the time
#
##
convertTime() {
    local ___resultvar=$1
    local myresult=0

    if [ -z $___resultvar ]
    then
        myresult=0
    else
        myresult=$(bc <<<"scale=2; ${___resultvar} / 1000 / 60")
    fi

    echo ${myresult}
}

##
# Description:
#   Check if the input argument
#
# Arguments:
#   $1 - Cpu User Time
#   
# Return:
#   return 0, if is empty otherwise the time
#
##
convertNanoToMin() {
    local ___resultvar=$1
    local myresult=0

    if [ -z $___resultvar ]
    then
        myresult=0
    else
        myresult=$(bc <<<"scale=2; ${___resultvar} / 1000000000 / 60")
    fi

    echo ${myresult}
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
    local serTime=0
    local deserTime=0
    local taskCpuUsrTime=0
    local gcTime=0

    cat ${fileToParse} | grep "LoadTime" > ${benchDir}/total_time.txt
    cat ${fileToParse} | grep "ParseTime" >> ${benchDir}/total_time.txt
    cat ${fileToParse} | grep "ComputationTime" >> ${benchDir}/total_time.txt
    cat ${fileToParse} | grep "SaveTime" >> ${benchDir}/total_time.txt

    pushd /opt/spark/spark-2.3.0-kolokasis/work/app-* 2>&1 >/dev/null

    for f in $(ls)
    do
        pushd $f 2>&1 >/dev/null
        
        # Calculation serialization and deserialization time
        serTime=$(cat stdout |grep "KryoSerializationStream::writeObject::time" \
            | awk -F ' ' '{sum+=$NF} END {print sum}')

        serTime=$(convertTime $serTime)

        deserTime=$(cat stdout |grep "KryoDeserializationStream::readObject::time" \
            | awk -F ' ' '{sum+=$NF} END {print sum}')

        deserTime=$(convertTime $deserTime)
        
        echo "SerTime_${f};${serTime}" >> /home1/public/kolokasis/sparkPersistentMemory/benchmarks/scripts/${benchDir}/total_time.txt
        echo "DeserTime_${f};${deserTime}" >> /home1/public/kolokasis/sparkPersistentMemory/benchmarks/scripts/${benchDir}/total_time.txt
        
        popd 2>&1 >/dev/null
    done

    # Return to the initial directory
    popd 2>&1 >/dev/null

    # Parse Garbage Collection Time
    for executorId in {0..6}
    do 
        gcTime=$(tail -n 1 ${benchDir}/gcTime_${executorId}.txt | awk '{print $NF}')
        
        echo "GCTime_${executorId};${gcTime}" >> ${benchDir}/total_time.txt
    done

    # Driver parse result
    # Calculation serialization and deserialization time
    #serTime=$(cat ${fileToParse} |grep "KryoSerializationStream" \
    #    | awk -F ' ' '{sum+=$NF} END {print sum}')
    ## serTime=$(bc <<<"scale=2; ${serTime} / 1000 / 60")
    #serTime=$(convertTime $serTime)

    #deserTime=$(cat ${fileToParse} |grep "KryoDeserializationStream" \
    #    | awk -F ' ' '{sum+=$NF} END {print sum}')
    ## deserTime=$(bc <<<"scale=2; ${deserTime} / 1000 / 60")
    #deserTime=$(convertTime $deserTime)

    #echo "SerTime_6;${serTime}" >> ${benchDir}/total_time.txt
    #echo "DeserTime_6;${deserTime}" >> ${benchDir}/total_time.txt

    # Remove driver output file
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
    echo "ML EXPERIMENTS"
    echo
    echo "      DEVICE   : $1"
    echo "      WORKLOAD : $2"
    echo
    echo -n "       Number of Partitions: "
    
}

getCPU() {
    local output=$1
    cat /proc/stat | grep "cpu " > ${output}
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
        # setConf "../spark-bench-legacy_${ID}/${benchmark}" ${partitions}

        echo -n "${partitions} "
        
        for ((i=0; i<${ITER}; i++))
        do
            # Create a directory for each run
            mkdir -p ${BENCH_DIR}/${benchmark}/run_${i}
            RUN_DIR="${BENCH_DIR}/${benchmark}/run_${i}"

            exportLogFile ${RUN_DIR} ${TYPE} ${DATA_SIZE} ${partitions}

            # JSTAT Statistics [background process]
            ./myjstat.sh ${RUN_DIR}/gcTime ${EXECUTORS} ${RUN_DIR}/perf.txt &

            # Drop caches
            sudo sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

            # IOSTAT Statistics [background process]
            iostat -xmt 1 > ${RUN_DIR}/iostat.txt & 

            # JVM Profiler - Executor [background process]
            #./jvmProfile.sh ${RUN_DIR}/flamegraph.svg &

            # Get current CPU utilization
            getCPU ${LOG_START}

            # Run benchmark and save output to tmp_out.txt
            ../spark-bench-legacy_${ID}/${benchmark}/bin/run.sh \
                > ${RUN_DIR}/tmp_out.txt
            
            # Get current CPU utilization
            # TODO Uncomment
            getCPU ${LOG_END}

            # Calculate cpu utilization
            ./cpuUtil.sh ${LOG_START} ${LOG_END} > ${RUN_DIR}/cpuUtil.txt
            mv ${LOG_START} ${RUN_DIR}/
            mv ${LOG_END} ${RUN_DIR}/

            # Kill all background processes
            killProcess
            
            # Parse the results of tmp_out.txt
            parseResult ${RUN_DIR} ${RUN_DIR}/tmp_out.txt
            
            # Save the total duration of the benchmark execution
            tail -n 1 ../spark-bench-legacy_${ID}/num/bench-report.dat \
                >> ${RUN_DIR}/total_time.txt

            # Clean executors log files
            # cleanWorkDirs
            mv /opt/spark/spark-2.3.0-kolokasis/work/app-* ${RUN_DIR}/
            mv /opt/spark/spark-2.3.0-kolokasis/logs/app-* ${RUN_DIR}/
            mv ~/gc_time_test ${RUN_DIR}/

            # Collect Spark Metrics Value
            #/opt/spark/spark-2.3.0-kolokasis/bin/spark-submit \
            #    sparkMeasureReport_1.py -o ${RUN_DIR}/sparkMetrics.csv

            # Move to workload result directory 
            # mv taskMetrics_1.serialized ${RUN_DIR}
        done
    done

    ENDTIME=$(date +%s)
    printEndMsg ${STARTTIME} ${ENDTIME}
done
