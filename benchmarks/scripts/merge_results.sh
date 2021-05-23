#!/usr/bin/env bash
###################################################
#
# file: merge_results.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-02-2021 
# @email:    kolokasis@ics.forth.gr
#
# Merge data from all configurations into one csv file
# Merge data from multiple runs
#
###################################################

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -d	Directory with results"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts ":d:h" opt
do
    case "${opt}" in
        d)
            RESULTS=${OPTARG}
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

pushd ${RESULTS}

for benchmark in $(ls)
do
	pushd ${benchmark}

	TOTAL_RUNS=0

	for run in $(ls)
	do
		TOTAL_RUNS=$(( ${COUNT} + 1))
		pushd ${run}

		HEADER=("TIME" "E" "0.1" "0.2" "0.4" "0.6" "0.8" "D" )
		EXEC=( "EXEC" )
		GC=( "GC" )
		SERDES=( "SERDES" )
		READS=( "READS(GB)" )
		WRITES=( "WRITES(GB)" )


		TOTAL_CONF=0

		for conf in $(ls)
		do
			TOTAL_CONF=$(( ${TOTAL_CONF} + 1 ))

			echo ${conf}/result.csv
			EXEC+=($(grep "TOTAL_TIME" ${conf}/result.csv | awk -F ',' '{print $2}'))
			GC+=($(grep GC ${conf}/result.csv | awk -F ',' '{sum += $2} END {print sum}'))
			SERDES+=($(grep "SER" ${conf}/result.csv | awk -F ',' '{print $2}'))
			READS+=($(grep "READS" ${conf}/diskstat.csv | awk -F ',' '{print $2}'))
			WRITES+=($(grep "WRITES" ${conf}/diskstat.csv | awk -F ',' '{print $2}'))
		done

		IFS=$','; echo "${HEADER[*]}" > $(pwd)/results.csv
		IFS=$','; echo "${EXEC[*]}" >> $(pwd)/results.csv
		IFS=$','; echo "${GC[*]}" >> $(pwd)/results.csv
		IFS=$','; echo "${SERDES[*]}" >> $(pwd)/results.csv
		IFS=$','; echo "${READS[*]}" >> $(pwd)/results.csv
		IFS=$','; echo "${WRITES[*]}" >> $(pwd)/results.csv

		popd

	done
	
	if [ ${TOTAL_RUNS} -gt 1 ]
	then
		for run in $(ls -d */)
		do
			paste -d , ${run}/results.csv >> tmp.csv
		done
		
		TOTAL_CONF=$(( ${TOTAL_CONF} + 1 ))

		EXEC=( "EXEC" )
		GC=( "GC" )
		SERDES=( "SERDES" )

		for (( i=2; i<=${TOTAL_CONF}; i++))
		do
			EXEC+=($(grep "EXEC" tmp.csv | awk -v c=$i -F',' '{sum+=$c; ++n} END { print sum/n }'))
			GC+=($(grep "GC" tmp.csv | awk -v c=$i -F',' '{sum+=$c; ++n} END { print sum/n }'))
			SERDES+=($(grep "SERDES" tmp.csv | awk -v c=$i -F',' '{sum+=$c; ++n} END { print sum/n }'))
		done

		IFS=$','; echo "${HEADER[*]}" > results.csv
		IFS=$','; echo "${EXEC[*]}" >> results.csv
		IFS=$','; echo "${GC[*]}" >> results.csv
		IFS=$','; echo "${SERDES[*]}" >> results.csv

		rm tmp.csv
	else
		cp run0/results.csv .
	fi
	popd
done
popd
