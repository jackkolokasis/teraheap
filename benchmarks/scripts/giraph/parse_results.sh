###################################################
#
# file: parse_results.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-02-2022
# @email:    kolokasis@ics.forth.gr
#
# Parse the results for the experiments
#
###################################################

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] "
    echo
    echo "Options:"
    echo "      -d  Directory with results"
    echo "      -t  Enable TeraCache"
    echo "      -h  Show usage"
    echo

    exit 1
}


# Check for the input arguments
while getopts "d:th" opt
do
    case "${opt}" in
        t)
            TC=true
            ;;
        d)
            RESULT_DIR="${OPTARG}"
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

BENCH_FILE=$(find ${RESULT_DIR}/ -name "benchmark-summary.log")
TOTAL_TIME=$(grep "execution process finished within" ${BENCH_FILE} | awk '{print $9}')

NUM_MGC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{print $7}')
MINOR_GC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{printf("%.2f",$8)}')
NUM_FGC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{print $9}')
MAJOR_GC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{printf("%.2f", $10)}')

# Caclulate the overheads in TC card table traversal, marking and adjust phases
if [ $TC ]
then
	TC_CT_TRAVERSAL=$(grep "TC_CT" ${RESULT_DIR}/teraCache.txt \
		| awk '{print $5}' \
		| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')
	HEAP_CT_TRAVERSAL=$(grep "HEAP_CT" ${RESULT_DIR}/teraCache.txt \
		| awk '{print $5}' \
		| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')
	PHASE0=$(grep "PHASE0" ${RESULT_DIR}/teraCache.txt  \
		| awk '{print $5}' \
		| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')
fi

PHASE1=$(grep "PHASE1" ${RESULT_DIR}/teraCache.txt \
	| awk '{print $5}' \
	| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')
PHASE2=$(grep "PHASE2" ${RESULT_DIR}/teraCache.txt \
	| awk '{print $5}' \
	| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')
PHASE3=$(grep "PHASE3" ${RESULT_DIR}/teraCache.txt \
	| awk '{print $5}' \
	| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')
PHASE4=$(grep "PHASE4" ${RESULT_DIR}/teraCache.txt \
	| awk '{print $5}' \
	| awk '{ sum += $1 } END {printf("%.2f", sum/1000.0)}')

# Caclulate the serialziation/deserialization overhead
# Make Kryo to Java for SQL
~/sparkPersistentMemory/benchmarks/profiler/perf-map-agent/FlameGraph/flamegraph.pl ${RESULT_DIR}/serdes.txt > ${RESULT_DIR}/profile.svg
SER_SAMPLES=$(grep "com/esotericsoftware/kryo/io/KryoDataOutput" ${RESULT_DIR}/profile.svg \
	| awk '{print $2}' \
	| sed 's/,//g' | sed 's/(//g' \
	| awk '{sum+=$1} END {print sum}')
DESER_SAMPLES=$(grep "com/esotericsoftware/kryo/io/KryoDataInput" ${RESULT_DIR}/profile.svg \
	| awk '{print $2}' \
	| sed 's/,//g' |sed 's/(//g' \
	| awk '{sum+=$1} END {print sum}')
APP_THREAD_SAMPLES=$(grep -w "java/lang/Thread.run" ${RESULT_DIR}/profile.svg \
	| awk '{print $2}' \
	| sed 's/,//g' \
	| sed 's/(//g' \
	| head -n 1)

NET_TIME=$(echo "${TOTAL_TIME} - ${MINOR_GC} - ${MAJOR_GC}" | bc -l)
SD_SAMPLES=$(echo "${SER_SAMPLES} + ${DESER_SAMPLES}" | bc -l)
SERDES=$(echo "scale=2; ${SD_SAMPLES} * ${NET_TIME} / ${APP_THREAD_SAMPLES}" | bc -l)

# Remove flamegraph
rm ${RESULT_DIR}/profile.svg

echo "---------,-------"						   > ${RESULT_DIR}/result.csv
echo "COMPONENT,TIME(s)"						   >> ${RESULT_DIR}/result.csv
echo "---------,-------"						   >> ${RESULT_DIR}/result.csv

echo "TOTAL_TIME,${TOTAL_TIME}"					   >> ${RESULT_DIR}/result.csv

echo "MINOR_GC,${MINOR_GC}"						   >> ${RESULT_DIR}/result.csv
echo "MAJOR_GC,${MAJOR_GC}"           			   >> ${RESULT_DIR}/result.csv

echo "TC_MINOR_GC,${TC_CT_TRAVERSAL}"              >> ${RESULT_DIR}/result.csv
echo "HEAP_MINOR_GC,${HEAP_CT_TRAVERSAL}"          >> ${RESULT_DIR}/result.csv
echo "PHASE0_FGC,${PHASE0}"                        >> ${RESULT_DIR}/result.csv
echo "PHASE1_FGC,${PHASE1}"                        >> ${RESULT_DIR}/result.csv
echo "PHASE2_FGC,${PHASE2}"                        >> ${RESULT_DIR}/result.csv
echo "PHASE3_FGC,${PHASE3}"                        >> ${RESULT_DIR}/result.csv
echo "PHASE4_FGC,${PHASE4}"                        >> ${RESULT_DIR}/result.csv
echo "SERDES,${SERDES}"		                       >> ${RESULT_DIR}/result.csv

echo "SER_SAMPLES,${SER_SAMPLES}"		            > ${RESULT_DIR}/serdes.csv
echo "DESER_SAMPLES,${DESER_SAMPLES}"		       >> ${RESULT_DIR}/serdes.csv
echo "APP_THREAD_SAMPLES,${APP_THREAD_SAMPLES}"    >> ${RESULT_DIR}/serdes.csv

if [ $TC ]
then
	grep "TOTAL_TRANS_OBJ" ${RESULT_DIR}/teraCache.txt    \
		| awk '{print $3","$5}' > ${RESULT_DIR}/statistics.csv

	grep "TOTAL_FORWARD_PTRS" ${RESULT_DIR}/teraCache.txt \
		| awk '{print $3","$5}' >> ${RESULT_DIR}/statistics.csv

	grep "TOTAL_BACK_PTRS" ${RESULT_DIR}/teraCache.txt \
		| awk '{print $3","$5}' >> ${RESULT_DIR}/statistics.csv

	grep "DUMMY" ${RESULT_DIR}/teraCache.txt \
		| awk '{sum+=$6} END {print "DUMMY_OBJ_SIZE(GB),"sum*8/1024/1024}' \
		>> ${RESULT_DIR}/statistics.csv

	grep "DISTRIBUTION" ${RESULT_DIR}/teraCache.txt |tail -n 1 \
		|awk '{print $5 " " $6 " " $7 " " $8 " " $9 " " $10 " " $11 " " $12" " $13 " " $14 " " $15}' \
		>> ${RESULT_DIR}/statistics.csv

	grep "TOTAL_OBJECTS_SIZE" ${RESULT_DIR}/teraCache.txt \
		| tail -n 1 \
		| awk '{printf("%.2f",$5*8/1024/1024/1024.0)}' >> ${RESULT_DIR}/statistics.csv
fi

# Read the Utilization from system.csv file
USR_UTIL_PER=$(grep "USR_UTIL" ${RESULT_DIR}/system.csv |awk -F ',' '{print $2}')
SYS_UTIL_PER=$(grep "SYS_UTIL" ${RESULT_DIR}/system.csv |awk -F ',' '{print $2}')
IO_UTIL_PER=$(grep "IOW_UTIL" ${RESULT_DIR}/system.csv |awk -F ',' '{print $2}')

# Convert CPU utilization to time 
USR_TIME=$( echo "scale=2; ${TOTAL_TIME} * ${USR_UTIL_PER} / 100" | bc -l )
SYS_TIME=$( echo "scale=2; ${TOTAL_TIME} * ${SYS_UTIL_PER} / 100" | bc -l )
IOW_TIME=$( echo "scale=2; ${TOTAL_TIME} * ${IO_UTIL_PER} / 100" | bc -l )

echo "---------,-------"	>> ${RESULT_DIR}/result.csv
echo "CPU,TIME(s)"			>> ${RESULT_DIR}/result.csv
echo "---------,-------"	>> ${RESULT_DIR}/result.csv
echo "USR_TIME,${USR_TIME}"	>> ${RESULT_DIR}/result.csv
echo "SYS_TIME,${SYS_TIME}"	>> ${RESULT_DIR}/result.csv
echo "IOW_TIME,${IOW_TIME}"	>> ${RESULT_DIR}/result.csv
echo "---------,-------"	>> ${RESULT_DIR}/result.csv
