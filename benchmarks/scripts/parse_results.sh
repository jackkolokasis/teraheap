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
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check the number of arguments
if [[ $# -ne 2 ]]; then
	usage
    exit 1
fi

# Check for the input arguments
while getopts "d:h" opt
do
    case "${opt}" in
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

TOTAL_TIME=$(tail -n 1 ${RESULT_DIR}/total_time.txt | awk '{split($0,a,","); print a[3]}')
MINOR_GC=$(tail -n 1 ${RESULT_DIR}/jstat.txt | awk '{print $8}')
MAJOR_GC=$(tail -n 1 ${RESULT_DIR}/jstat.txt | awk '{print $10}')
TC_CT_TRAVERSAL=$(grep "MINOR_GC" ${RESULT_DIR}/teraCache.txt | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
TC_MARK=$(grep "TC_MARK" ${RESULT_DIR}/teraCache.txt | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')
TC_ADJUST=$(grep "TC_ADJUST" ${RESULT_DIR}/teraCache.txt | awk '{print $5}' | awk '{ sum += $1 } END {print sum }')

echo "COMPONENT,TIME(s)" > ${RESULT_DIR}/result.csv
echo "TOTAL_TIME,${TOTAL_TIME}" >> ${RESULT_DIR}/result.csv

echo "MINOR_GC,${MINOR_GC}" >> ${RESULT_DIR}/result.csv
echo "TC_MINOR_GC,${TC_CT_TRAVERSAL}" >> ${RESULT_DIR}/result.csv

echo "MAJOR_GC,${MAJOR_GC}" >> ${RESULT_DIR}/result.csv
echo "TC_MARK,${TC_MARK}" >> ${RESULT_DIR}/result.csv
echo "TC_ADJUST,${TC_ADJUST}" >> ${RESULT_DIR}/result.csv

grep "TOTAL_TRANS_OBJ" ${RESULT_DIR}/teraCache.txt | awk '{print $3","$5}' > ${RESULT_DIR}/statistics.csv
grep "TOTAL_FORWARD_PTRS" ${RESULT_DIR}/teraCache.txt | awk '{print $3","$5}' >> ${RESULT_DIR}/statistics.csv
grep "TOTAL_BACK_PTRS" ${RESULT_DIR}/teraCache.txt | awk '{print $3","$5}' >> ${RESULT_DIR}/statistics.csv
