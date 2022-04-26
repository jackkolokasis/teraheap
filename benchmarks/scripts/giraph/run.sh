#!/usr/bin/env bash

###################################################
#
# file: run.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  26-02-2022 
# @email:    kolokasis@ics.forth.gr
#
# Scrpt to run Giraph benchmarks
#
###################################################

. ./conf.sh
# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-h]"
    echo
    echo "Options:"
    echo "      -n  Number of Runs"
    echo "      -o  Output Path"
    echo "      -c  Enable TeraCache"
    echo "      -s  Enable serialization"
    echo "      -p  Enable perf tool"
    echo "      -f  Enable profiler tool"
    echo "      -f  Enable profiler tool"
    echo "      -a  Run experiments with high bench"
    echo "      -b  Run experiments with custom benchmark"
    echo "      -j  Enable metrics for JIT compiler"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check if the last command executed succesfully
#
# if executed succesfully, print SUCCEED
# if executed with failures, print FAIL and exit
check () {
    if [ $1 -ne 0 ]
    then
        echo -e "  $2 \e[40G [\e[31;1mFAIL\e[0m]" >> $LOG
        exit
    else
        echo -e "  $2 \e[40G [\e[32;1mSUCCED\e[0m]" >> $LOG 
    fi
}

##
# Description: 
#   Start Hadoop, Yarn, and Zookeeper
#
# Arguments
#	$1: Indicateds if we ran S/D experiments, TeraHeap otherwise
##
start_hadoop_yarn_zkeeper() {
	local is_ser=$1
	local jvm_opts=""

	if [ -z ${is_ser} ]
	then
		local tc_size=$(( (900 - ${HEAP}) * 1024 * 1024 * 1024 ))

		jvm_opts="\t\t<value>-XX:-ClassUnloading -XX:+UseParallelGC "
		jvm_opts+="-XX:-UseParallelOldGC -XX:ParallelGCThreads=${GC_THREADS} -XX:+EnableTeraCache "
		jvm_opts+="-XX:TeraCacheSize=${tc_size} -Xmx900g -Xms${HEAP}g "
		jvm_opts+="-XX:TeraCacheThreshold=0 -XX:-UseCompressedOops " 
		jvm_opts+="-XX:-UseCompressedClassPointers -XX:+TeraCacheStatistics "
		jvm_opts+="-Xlogtc:${BENCHMARK_SUITE//'/'/\\/}\/report\/teraCache.txt "
		jvm_opts+="-XX:TeraStripeSize=32768 -XX:+ShowMessageBoxOnError<\/value>"
	else
		jvm_opts="\t\t<value>-Xms${HEAP}g -Xmx${HEAP}g -XX:-ClassUnloading -XX:+UseParallelGC "
		jvm_opts+="-XX:-UseParallelOldGC -XX:ParallelGCThreads=${GC_THREADS} -XX:-ResizeTLAB "
		jvm_opts+="-XX:-UseCompressedOops -XX:-UseCompressedClassPointers "
		jvm_opts+="-XX:+TimeBreakDown -Xlogtime:${BENCHMARK_SUITE//'/'/\\/}\/report\/teraCache.txt<\/value>"
	fi

	# Yarn child executor jvm flags
	sed '/java.opts/{n;s/.*/'"${jvm_opts}"'/}' -i ${HADOOP}/etc/hadoop/mapred-site.xml
	retValue=$?
	message="Update jvm flags" 
	check ${retValue} "${message}"

	# Format Hadoop
	${HADOOP}/bin/hdfs namenode -format >> $LOG 2>&1
	retValue=$?
	message="Format Hadoop" 
	check ${retValue} "${message}"

	# Start hadoop, yarn, and zookeeper
	${HADOOP}/sbin/start-dfs.sh >> $LOG 2>&1
	retValue=$?
	message="Start HDFS" 
	check ${retValue} "${message}"

	${HADOOP}/sbin/start-yarn.sh >> $LOG 2>&1
	retValue=$?
	message="Start Yarn" 
	check ${retValue} "${message}"

	${ZOOKEEPER}/bin/zkServer.sh start >> $LOG 2>&1
	retValue=$?
	message="Start Zookeeper" 
	check ${retValue} "${message}"
}

##
# Description: 
#   Stop Hadoop, Yarn, and Zookeeper
##
stop_hadoop_yarn_zkeeper() {
	${ZOOKEEPER}/bin/zkServer.sh stop >> $LOG 2>&1
	retValue=$?
	message="Stop Zookeeper" 
	check ${retValue} "${message}"
	
	${HADOOP}/sbin/stop-yarn.sh >> $LOG 2>&1
	retValue=$?
	message="Stop Yarn" 
	check ${retValue} "${message}"

	${HADOOP}/sbin/stop-dfs.sh >> $LOG 2>&1
	retValue=$?
	message="Stop HDFS" 
	check ${retValue} "${message}"

	rm -rf /mnt/datasets/hadoop
	rm -f /mnt/fmap/file.txt
	rm -rf /mnt/fmap/partitions
}

##
# Description: 
#   Stop perf monitor statistics with signal interupt (SIGINT)
#
##
stop_perf() {
	local perfPID=$(pgrep perf)
	
	# Kill all perf process
    for perf_id in ${perfPID}
    do
        kill -2 ${perf_id}
    done
}

##
# Description: 
#   Kill running background processes (jstat, serdes)
#
# Arguments:
#   $1 - Restart Spark
##
kill_back_process() {
    local jstatPID=$(pgrep jstat)
	local serdesPID=$(pgrep serdes)
	local perfPID=$(pgrep perf)
 
    # Kill all jstat process
    for jstat_pid in ${jstatPID}
    do
        kill -KILL ${jstat_pid}
    done
    
	# Kill all serdes process
    for serdes_pid in ${serdesPID}
    do
        kill -KILL ${serdes_pid}
    done
	
	# Kill all perf process
    for perf_id in ${perfPID}
    do
        kill -KILL ${perf_id}
    done

	kill -KILL $(jps | grep "MRApp" | awk '{print $1}')
	kill -KILL $(jps | grep "YarnChild" | awk '{print $1}')
    
    if [[ $1 == "restart" ]]
    then
        stop_hadoop_yarn_zkeeper
        start_hadoop_yarn_zkeeper
    fi
}

##
# Description: 
#   Remove executors log files
##
cleanWorkDirs() {
	rm -rf ${BENCHMARK_SUITE}/report/*
}

##
# Description
#	Enable perf events
##
enable_perf_event() {
	sudo sh -c 'echo -1 >/proc/sys/kernel/perf_event_paranoid'
	retValue=$?
	message="Enable perf events" 
	check ${retValue} "${message}"
}

##
# Description
#	Update number of compute threads in configuration
#
# Arguments
#	$1: Benchmark
#	$2: Indicate if we run S/D experiment, TeraHeap otherwise
##
update_conf() {
	local bench=$1
	local heap_size=$(( $HEAP * 1024 ))
	local is_ser=$2

	# Set benchmark
	sed -i '/benchmark.custom.algorithms/c\benchmark.custom.algorithms = '"$bench" \
		${BENCHMARK_CONFIG}/benchmarks/custom.properties 
	
	# Set heap size
	sed -i '/memory-size/c\platform.giraph.job.memory-size: '"${heap_size}" \
		${BENCHMARK_CONFIG}/platform.properties
	sed -i '/heap-size/c\platform.giraph.job.heap-size: '"${heap_size}" \
		${BENCHMARK_CONFIG}/platform.properties

	# Set number of compute threads
	sed -i '/numComputeThreads/c\platform.giraph.options.numComputeThreads: '"${COMPUTE_THREADS}" \
		${BENCHMARK_CONFIG}/platform.properties

	if  [ -z ${is_ser} ]
	then
		# Comment these lines in the configuration
		sed -e '/platform.giraph.options.useOutOfCoreGraph/s/^/#/' \
			-i ${BENCHMARK_CONFIG}/platform.properties
 
		sed -e '/platform.giraph.options.partitionsDirectory/s/^/#/' \
			-i ${BENCHMARK_CONFIG}/platform.properties
	else
		# Uncomment these lines in the configuration
		sed -e '/platform.giraph.options.useOutOfCoreGraph/s/^#//' \
			-i ${BENCHMARK_CONFIG}/platform.properties
 
		sed -e '/platform.giraph.options.partitionsDirectory/s/^#//' \
			-i ${BENCHMARK_CONFIG}/platform.properties
	fi
}

##
# Description
#	Create, mount and fill ramdisk to reduce server available memory. 
#
# Arguments:
#	$1: Iteration
#	
##
create_ramdisk() {
	local iter=$1

	if [ ${RAMDISK} -eq 0 ] || [ ${iter} -gt 0 ]
	then
		return
	fi

	# Check if ramdisk_create_and_mount.sh exists
	if [ ! -f "${RAMDISK_SCRIPT_DIR}/ramdisk_create_and_mount.sh" ]
	then
		cp ramdisk_create_and_mount.sh ${RAMDISK_SCRIPT_DIR}/
	fi
	
	cd ${RAMDISK_SCRIPT_DIR}

	# If a previous ramdisk exist then remove it
	if [ ! -z "$(lsmod | grep "brd")"]
	then
		sudo ./ramdisk_create_and_mount.sh -d >> ${LOG} 2>&1
	fi

	# Create the new ramdisk
	local MEM=$(( ${RAMDISK} * 1024 * 1024 ))
	sudo ./ramdisk_create_and_mount.sh -m ${MEM} -c >> ${LOG} 2>&1

	cd - >> ${LOG} 2>&1

	cd ${RAMDISK_DIR}

	# Fill the ramdisk
	MEM=$(( ${RAMDISK} * 1024 ))
	dd if=/dev/zero of=file.txt bs=1M count=${MEM} >> ${LOG} 2>&1

	cd - >> ${LOG} 2>&1
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
    echo "============================================="
    echo 
    echo "EXPERIMENTS"
    echo
    echo "      DEVICE   : $1"
    echo "      WORKLOAD : $2"
    echo -n "      ITERATION: "
}

##
# Description: 
#   Console Message
#
# Arguments:
#   $1 - Iteration
##
printMsgIteration() {
    echo -n "$1 "
}

##
# Description:
#	Average Time
#
# Arguments:
#	$1: Result Directory
#	$2: Iterations 
#	$3: Indicate that with S/D, TeraHeap otherwise
##
calculate_avg() {
	local bench_dir=$1 iter=$2
	local exec_time minor_gc major_gc serdes
	local avg_exec_time=0 avg_minor_gc_time=0 avg_major_gc_time=0 avg_sd_time=0
	local other=0 max min max_index min_index sum

	cd ${bench_dir}

	for d in $(ls -l | grep '^d' | awk '{print $9}')
	do
		exec_time+=($(grep -w "TOTAL_TIME" ${d}/result.csv \
			| awk -F ',' '{if ($2 == "") print 0; else print $2}'))
		minor_gc+=($(grep -w "MINOR_GC" ${d}/result.csv \
			| awk -F ',' '{if ($2 == "") print 0; else print $2}'))
		major_gc+=($(grep -w "MAJOR_GC" ${d}/result.csv \
			| awk -F ',' '{if ($2 == "") print 0; else print $2}'))
		serdes+=($(grep -w "SERDES" ${d}/result.csv \
			| awk -F ',' '{if ($2 == "") print 0; else print $2}'))
	done

	# Remove outliers (maximum and the minimum) 
	max=$(echo "${exec_time[*]}" | tr ' ' '\n' | sort -nr | head -n 1)
	min=$(echo "${exec_time[*]}" | tr ' ' '\n' | sort -nr | tail -n 1)

	# Finda the max and min values indexes
	max_index=$(echo ${exec_time[*]} | tr ' ' '\n' | awk '/'"${max}"'/ {print NR-1}')
	min_index=$(echo ${exec_time[*]} | tr ' ' '\n' | awk '/'"${min}"'/ {print NR-1}')

	# Remove the max and min values using indexes from all arrays
	unset 'exec_time[$max_index]'
	unset 'exec_time[$min_index]'
	unset 'minor_gc[$max_index]'
	unset 'minor_gc[$min_index]'
	unset 'major_gc[$max_index]'
	unset 'major_gc[$min_index]'
	unset 'serdes[$max_index]'
	unset 'serdes[$min_index]'

	sum=$(echo "scale=2; ${exec_time[@]/%/ +} 0" | bc -l)
	avg_exec_time=$(echo "scale=2; ${sum}/(${iter} - 2)" | bc -l)

	sum=$(echo "scale=2; ${minor_gc[@]/%/ +} 0" | bc -l)
	avg_minor_gc_time=$(echo "scale=2; ${sum}/(${iter} - 2)" | bc -l)

	sum=$(echo "scale=2; ${major_gc[@]/%/ +} 0" | bc -l)
	avg_major_gc_time=$(echo "scale=2; ${sum}/(${iter} - 2)" | bc -l)
	
	sum=$(echo "scale=2; ${serdes[@]/%/ +} 0" | bc -l)
	avg_sd_time=$(echo 'scale=2; x='$sum'/('$iter' - 2); if(x<1){"0"}; x' | bc -l)
	other=$(echo "scale=2; ${avg_exec_time} - ${avg_major_gc_time} - ${avg_major_gc_time} - ${avg_sd_time}" | bc -l)

	echo "---------,-------"				   > time.csv
	echo "COMPONENT,TIME(s)"				  >> time.csv
	echo "---------,-------"				  >> time.csv
	echo "AVG_TOTAL_TIME,${avg_exec_time}"	  >> time.csv

	echo "AVG_OTHER,${other}"           	  >> time.csv
	echo "AVG_MINOR_GC,${avg_minor_gc_time}"  >> time.csv
	echo "AVG_MAJOR_GC,${avg_major_gc_time}"  >> time.csv
	echo "AVG_SERDES,${avg_sd_time}"          >> time.csv

	cd - >> ${LOG} 2>&1
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
    echo "============================================="
    echo
}

# Check for the input arguments
while getopts ":n:o:m:cspkajfdbh" opt
do
    case "${opt}" in
        n)
            ITER=${OPTARG}
            ;;
        o)
            OUTPUT_PATH=${OPTARG}
            ;;
        k)
            kill_back_process
            exit 1
            ;;
		c)
			TH=true
			;;
		s)
			SERDES=true
			;;
		p)
			PERF_TOOL=true
			;;
		j)
			JIT=true
			;;
		f)
			PROFILER=true
			;;
		d)
			FASTMAP=true
			;;
		m)
			BENCH_DIR=${OPTARG}
			calculate_avg ${BENCH_DIR} ${ITER}
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

enable_perf_event

# Run each benchmark
for benchmark in "${BENCHMARKS[@]}"
do
    printStartMsg ${DEV_TH} ${benchmark}
	STARTTIME=$(date +%s)

	mkdir -p ${OUT}/${benchmark}
        
	# For every iteration
	for ((i=0; i<${TOTAL_CONFS}; i++))
	do
		mkdir -p ${OUT}/${benchmark}/conf${i}

		# For every configuration
		for ((j=0; j<${ITER}; j++))
		do
			printMsgIteration $j

			mkdir -p ${OUT}/${benchmark}/conf${i}/run${j}
			RUN_DIR="${OUT}/${benchmark}/conf${i}/run${j}"

			stop_hadoop_yarn_zkeeper

			# Prepare devices for Zookeeper and TeraCache accordingly
			if [ $SERDES ]
			then
				./dev_setup.sh
			else
				./dev_setup.sh -t
			fi

			start_hadoop_yarn_zkeeper ${SERDES}

			create_ramdisk ${j} 

			update_conf ${benchmark} ${SERDES}

			if [ -z "$JIT" ]
			then
				# Collect statics only for the garbage collector
				./jstat.sh ${RUN_DIR} ${EXECUTORS} 0 &
			else
				# Collect statics for garbage collector and JIT
				./jstat.sh ${RUN_DIR} ${EXECUTORS} 1 &
			fi
			
			if [ $PERF_TOOL ]
			then
				# Count total cache references, misses and pagefaults
				./perf.sh ${RUN_DIR}/perf.txt ${EXECUTORS} &
			fi

			./serdes.sh ${RUN_DIR}/serdes.txt ${EXECUTORS} &
			
			# Enable profiler
			if [ ${PROFILER} ]
			then
				./profiler.sh ${RUN_DIR}/profile.svg ${EXECUTORS} &
			fi

			# Drop caches
			sudo sync && echo 3 | sudo tee /proc/sys/vm/drop_caches >> $LOG 2>&1

            # System statistics start
			~/system_util/start_statistics.sh -d ${RUN_DIR}

			cd ${BENCHMARK_SUITE}

			# Run benchmark and save output to tmp_out.txt
			./bin/sh/run-benchmark.sh >> ${LOG} 2>&1

			cd - >> ${LOG} 2>&1
				
			if [ $PERF_TOOL ]
			then
				# Stop perf monitor
				stop_perf
			fi
            
            # System statistics stop
			~/system_util/stop_statistics.sh -d ${RUN_DIR}

			# Parse cpu and disk statistics results
			~/system_util/extract-data.sh -r ${RUN_DIR} -d ${DEV_TH} \
				-d ${DEV_HDFS} -d ${DEV_ZK} >> ${LOG} 2>&1

			# Copy the confifuration to the directory with the results
			cp ./conf.sh ${RUN_DIR}/

			cp -r $BENCHMARK_SUITE/report/* ${RUN_DIR}/

			rm -rf $BENCHMARK_SUITE/report/*

			if [ $TH ]
			then
				./parse_results.sh -d ${RUN_DIR} -t  >> ${LOG} 2>&1
			else
				./parse_results.sh -d ${RUN_DIR} >> ${LOG} 2>&1
			fi

			# Check if the run completed succesfully. If the run fail then retry
			# to run the same iteration
			check=$(grep "TOTAL_TIME" ${RUN_DIR}/result.csv | awk -F ',' '{print $2}')
			if [ -z ${check} ]  
			then
				j=$(($j - 1))
			fi
		done

		# Calculate Average
		#calculate_avg "${OUT}/${benchmark}/conf${i}" ${ITER}
	done

	ENDTIME=$(date +%s)
	printEndMsg ${STARTTIME} ${ENDTIME}
done

exit
