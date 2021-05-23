#!/usr/bin/env bash

###################################################
#
# file: run.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  20-01-2021 
# @email:    kolokasis@ics.forth.gr
#
# Run dacapo benchmark suite
#
###################################################

# All the benchmarks in dacabo
#	Batik benchmark always failed
#   jme failed
#   eclipse failed
#   jme
#   kafka, tradebeans tradesoap

BENCHMARKS=( avrora biojava cassandra fop graphchi h2o jython \
	luindex lusearch sunflow tomcat xalan zxing )

#DACAPO_JAR="../dacapobench/dacapo-evaluation-git+309e1fa-java8.jar"

DACAPO_JAR="./../dacapobench/dacapo-9.12-MR1-bach.jar"

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-h]"
    echo
    echo "Options:"
    echo "      -j  JAVA Path"
    echo "      -o  Output Path"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts "o:j:n:wh" opt
do
    case "${opt}" in
		j)
			JAVA=${OPTARG}
			;;
        o)
            OUTPUT_PATH=${OPTARG}
            ;;
		n)
			ITER=${OPTARG}
			;;
		w)
			WARMUP=true
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

for ((i=0; i<${ITER}; i++))
do
	mkdir -p ${OUT}/run${i}
	RUN_DIR=${OUT}/run${i}

	# Header file for results
	echo "Benchmark,Time(msec)" > ${RUN_DIR}/result.csv

	for bench in "${BENCHMARKS[@]}"
	do
		echo "${bench}"
		${JAVA} \
			-XX:+UseParallelGC \
			-XX:ParallelGCThreads=16 \
			-XX:-UseParallelOldGC \
			-Xmx50g \
			-Xms50g \
			-Xmn5g \
			-jar ${DACAPO_JAR} -n 50 ${bench} >> ${RUN_DIR}/file.err 2>&1 >> ${RUN_DIR}/file.out

	#	${JAVA} \
	#		-XX:+UseParallelGC \
	#		-XX:ParallelGCThreads=16 \
	#		-XX:-UseParallelOldGC \
	#		-XX:+EnableTeraCache \
	#		-XX:TeraCacheSize=1073741824 \
	#		-Xmx50g \
	#		-Xms49g \
	#		-Xmn5g \
	#		-jar ${DACAPO_JAR} ${bench} -n 10 >> ${RUN_DIR}/file.err 2>&1 >> ${RUN_DIR}/file.out

	# Parse Dacapo Results
	if [ ${WARMUP} ]
	then
		MIN=$(grep ${bench} ${RUN_DIR}/file.err | grep "msec" | tail -n 11 | head -n 10 | awk '{print $9}' | sort -nk 1 | head -n 1)
		echo "${bench},${MIN}" >> ${RUN_DIR}/result.csv
	else
		echo "Benchmark,Time(msec)" > ${RUN_DIR}/result.csv
		grep "PASSED" ${RUN_DIR}/file.err | awk '{ print $4","$7}' >> ${RUN_DIR}/result.csv
	fi
done
done
