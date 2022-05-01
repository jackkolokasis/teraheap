#!/usr/bin/env bash

###################################################
#
# file: update_conf.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-02-2021 
# @email:    kolokasis@ics.forth.gr
#
# Scrpt to setup the configuration for experiments
#
###################################################

# Print error/usage script message
usage() {
    echo
    echo "Usage:"
    echo -n "      $0 [option ...] [-k][-h]"
    echo
    echo "Options:"
    echo "      -m  Minimum Heap Size"
    echo "      -f  Spark Memory Fraction"
    echo "      -s  Storage Level"
    echo "      -r  Ramdisk size"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts ":m:f:s:r:c:b:h" opt
do
    case "${opt}" in
        m)
            MIN_HEAP=${OPTARG}
            ;;
        f)
            FRACTION=${OPTARG}
            ;;
        s)
            S_LEVEL=${OPTARG}
            ;;
        r)
            RAMDISK=${OPTARG}
            ;;
        c)
            CORES=${OPTARG}
            ;;
        b)
            CUSTOM_BENCHMARK=${OPTARG}
            ;;
        h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

# Enter to spark configuration
cd /opt/spark/spark-2.3.0-kolokasis/conf

# Change the worker cores
sed -i '/SPARK_WORKER_CORES/c\SPARK_WORKER_CORES='"${CORES}" spark-env.sh

# Change the worker memory
sed -i '/SPARK_WORKER_MEMORY/c\SPARK_WORKER_MEMORY='"${MIN_HEAP}"'g' spark-env.sh

# Change the minimum heap size
# Change only the first -Xms 
# sed -i -e '0,/-Xms[0-9]*g/ s/-Xms[0-9]*g/-Xms'"${MIN_HEAP}"'g/' spark-defaults.conf

# Change the spark.memory.storageFraction
sed -i '/storageFraction/c\spark.memory.storageFraction '"${FRACTION}" spark-defaults.conf

cd -

if [ $CUSTOM_BENCHMARK == "false" ]
then
	# Enter the spark-bechmarks
	cd ../spark-bench/conf/

	# Change spark benchmarks configuration execur memory
	sed -i '/SPARK_EXECUTOR_MEMORY/c\SPARK_EXECUTOR_MEMORY='"${MIN_HEAP}"'g' env.sh

	# Change spark benchmarks configuration executor core
	sed -i '/SPARK_EXECUTOR_CORES/c\SPARK_EXECUTOR_CORES='"${CORES}" env.sh

	# Change storage level
	sed -i '/STORAGE_LEVEL/c\STORAGE_LEVEL='"${S_LEVEL}" env.sh

	cd -
fi

if [ ${RAMDISK} -ne 0 ]
then
	cd /tmp

	# Remove the previous ramdisk
	sudo ./ramdisk_create_and_mount.sh -d

	# Create the new ramdisk
	MEM=$(( ${RAMDISK} * 1024 * 1024 ))
	sudo ./ramdisk_create_and_mount.sh -m ${MEM} -c

	cd /mnt/ramdisk

	# Fill the ramdisk
	MEM=$(( ${RAMDISK} * 1024 ))
	dd if=/dev/zero of=file.txt bs=1M count=${MEM}

	cd -
fi

exit
