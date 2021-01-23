#!/usr/bin/env bash

###################################################
#
# file: run.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  20-01-2021 
# @email:    kolokasis@ics.forth.gr
#
# Scrpt to run the experiments
#
###################################################

BENCHMARKS=("LogisticRegression")
EXECUTORS=1

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -n  Number of Runs"
    echo "      -t  Type of Storage Device (e.g SSD, NVMe, NVM)"
    echo "      -o  Output Path"
    echo "      -k  Kill iostat and jstat processes"
    echo "      -h  Show usage"
    echo

    exit 1
}

##
# Description: 
#   Start Spark
##
start_spark() {
    /opt/spark/spark-2.3.0-kolokasis/sbin/start-all.sh
}

##
# Description: 
#   Stop Spark
##
stop_spark() {
    /opt/spark/spark-2.3.0-kolokasis/sbin/stop-all.sh
}

##
# Description: 
#   Kill running background processes (iostat, jstat)
#
# Arguments:
#   $1 - Restart Spark
##
kill_back_process() {
    local iostatPID=$(pgrep iostat)
    local myjstatPID=$(pgrep myjstat)
 
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
    
    if [[ $1 == "restart" ]]
    then
        stopSpark
        startSpark
    fi
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
    echo "EXPERIMENTS"
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
        o)
            OUTPUT_PATH=${OPTARG}
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

# Create directory for the results if do not exist
TIME=$(date +"%T-%d-%m-%Y")

OUT="${OUTPUT_PATH}_${TIME}"
mkdir -p ${OUT}

# Run each benchmark
for benchmark in "${BENCHMARKS[@]}"
do
    printStartMsg ${TYPE} ${benchmark}
	STARTTIME=$(date +%s)

	mkdir -p ${OUT}/${benchmark}
        
	for ((i=0; i<${ITER}; i++))
	do
		mkdir -p ${OUT}/${benchmark}/run${i}
		RUN_DIR="${OUT}/${benchmark}/run${i}"

		# Garbage collector statistics
		./jstat.sh ${RUN_DIR}/jstat.txt ${EXECUTORS} &

		# Drop caches
		sudo sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

		# System statistics start
		~/system_util/start_statistics.sh -d ${RUN_DIR}
		
		# Run benchmark and save output to tmp_out.txt
		../spark-bench/${benchmark}/bin/run.sh \
			> ${RUN_DIR}/tmp_out.txt

		# System statistics stop
		~/system_util/stop_statistics.sh -d ${RUN_DIR}

		# Parse cpu and disk statistics results
		~/system_util/extract-data.sh -d ${RUN_DIR}

		# Kill all background processes
		# killProcess

		# Save the total duration of the benchmark execution
		tail -n 1 ../spark-bench/num/bench-report.dat \
			>> ${RUN_DIR}/total_time.txt
            
		TC_METRICS=$(ls -td /opt/spark/spark-2.3.0-kolokasis/work/* | head -n 1)
		cp ${TC_METRICS}/0/teraCache.txt ${RUN_DIR}/

		./parse_results.sh -d ${RUN_DIR}
	done
    
	ENDTIME=$(date +%s)
    printEndMsg ${STARTTIME} ${ENDTIME}
done
