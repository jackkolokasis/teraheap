###################################################
#
# file: parse_results.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  20-01-2021 
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
    echo "      -s  Enable serdes"
    echo "      -a  Enable hibench suite"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts "d:tsah" opt
do
    case "${opt}" in
        s)
            SER=true
            ;;
        t)
            TC=true
            ;;
        a)
            HIGH_BENCH=true
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

if [ $HIGH_BENCH ]
then
	TOTAL_TIME=$(tail -n 1 ${RESULT_DIR}/total_time.txt | awk '{print $5}')
else
	TOTAL_TIME=$(tail -n 1 ${RESULT_DIR}/total_time.txt | awk '{split($0,a,","); print a[3]}')
fi

NUM_MGC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{print $7}')
MINOR_GC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{print $8}')
NUM_FGC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{print $9}')
MAJOR_GC=$(tail -n 1 ${RESULT_DIR}/jstat.txt        | awk '{print $10}')

# Caclulate the overheads in TC card table traversal, marking and adjust phases
if [ $TC ]
then
	TC_CT_TRAVERSAL=$(grep "TC_CT" ${RESULT_DIR}/teraCache.txt | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
	HEAP_CT_TRAVERSAL=$(grep "HEAP_CT" ${RESULT_DIR}/teraCache.txt | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
	PHASE1=$(grep "PHASE1" ${RESULT_DIR}/teraCache.txt          | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
	PHASE2=$(grep "PHASE2" ${RESULT_DIR}/teraCache.txt          | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
	PHASE3=$(grep "PHASE3" ${RESULT_DIR}/teraCache.txt          | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
	PHASE4=$(grep "PHASE4" ${RESULT_DIR}/teraCache.txt          | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
fi

# Caclulate the serialziation/deserialization overhead
~/sparkPersistentMemory/benchmarks/profiler/perf-map-agent/FlameGraph/flamegraph.pl ${RESULT_DIR}/serdes.txt > ${RESULT_DIR}/profile.svg
SER_SAMPLES=$(grep "org/apache/spark/serializer/KryoSerializationStream.writeObject" ${RESULT_DIR}/profile.svg \
	| awk '{print $2}' \
	| sed 's/,//g' | sed 's/(//g' \
	| awk '{sum+=$1} END {print sum}')
DESER_SAMPLES=$(grep "org/apache/spark/serializer/KryoDeserializationStream.readObject" ${RESULT_DIR}/profile.svg \
	| awk '{print $2}' \
	| sed 's/,//g' |sed 's/(//g' \
	| awk '{sum+=$1} END {print sum}')
APP_THREAD_SAMPLES=$(grep -w "java/lang/Thread.run" ${RESULT_DIR}/profile.svg | awk '{print $2}' | sed 's/,//g' | sed 's/(//g')

NET_TIME=$(echo "${TOTAL_TIME} - ${MINOR_GC} - ${MAJOR_GC}" | bc -l)
SD_SAMPLES=$(echo "${SER_SAMPLES} + ${DESER_SAMPLES}" | bc -l)

SERDES=$(echo "${SD_SAMPLES} * ${NET_TIME} / ${APP_THREAD_SAMPLES}" | bc -l)

# Remove flamegraph
rm ${RESULT_DIR}/profile.svg

echo "COMPONENT,TIME(s)"               > ${RESULT_DIR}/result.csv
echo "TOTAL_TIME,${TOTAL_TIME}"       >> ${RESULT_DIR}/result.csv

echo "MINOR_GC,${MINOR_GC}"           >> ${RESULT_DIR}/result.csv
echo "MAJOR_GC,${MAJOR_GC}"           >> ${RESULT_DIR}/result.csv

echo "TC_MINOR_GC,${TC_CT_TRAVERSAL}" >> ${RESULT_DIR}/result.csv
echo "HEAP_MINOR_GC,${HEAP_CT_TRAVERSAL}" >> ${RESULT_DIR}/result.csv
echo "PHASE1_FGC,${PHASE1}"           >> ${RESULT_DIR}/result.csv
echo "PHASE2_FGC,${PHASE2}"           >> ${RESULT_DIR}/result.csv
echo "PHASE3_FGC,${PHASE3}"           >> ${RESULT_DIR}/result.csv
echo "PHASE4_FGC,${PHASE4}"           >> ${RESULT_DIR}/result.csv
echo "SERSES,${SERDES}"		          >> ${RESULT_DIR}/result.csv

echo "SER_SAMPLES,${SER_SAMPLES}"		         > ${RESULT_DIR}/serdes.csv
echo "DESER_SAMPLES,${DESER_SAMPLES}"		     >> ${RESULT_DIR}/serdes.csv
echo "APP_THREAD_SAMPLES,${APP_THREAD_SAMPLES}"  >> ${RESULT_DIR}/serdes.csv

if [ $TC ]
then
	grep "TOTAL_TRANS_OBJ" ${RESULT_DIR}/teraCache.txt    | awk '{print $3","$5}' > ${RESULT_DIR}/statistics.csv
	grep "TOTAL_FORWARD_PTRS" ${RESULT_DIR}/teraCache.txt | awk '{print $3","$5}' >> ${RESULT_DIR}/statistics.csv
	grep "TOTAL_BACK_PTRS" ${RESULT_DIR}/teraCache.txt    | awk '{print $3","$5}' >> ${RESULT_DIR}/statistics.csv
fi
