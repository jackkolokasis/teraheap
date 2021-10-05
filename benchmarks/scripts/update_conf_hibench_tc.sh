#!/usr/bin/env bash

###################################################
#
# file: update_conf_tc.sh
#
# @Author:   Iacovos G. Kolokasis
# @Version:  27-02-2021 
# @email:    kolokasis@ics.forth.gr
#
# Scrpt to setup the configuration for experiments
# for teracache
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
    echo "      -t  Set spark.teracache.heap.size"
    echo "      -h  Show usage"
    echo

    exit 1
}

# Check for the input arguments
while getopts ":m:f:s:r:t:n:c:h" opt
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
        t)
            TERACACHE=${OPTARG}
            ;;
		n)
			NEW_GEN=${OPTARG}
            ;;
		c)
			CORES=${OPTARG}
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
sed -i '/SPARK_WORKER_MEMORY/c\SPARK_WORKER_MEMORY='"${TERACACHE}"'g' spark-env.sh

cd -

#################
cd /home1/public/kolokasis/HiBench/conf

sed -i '/spark.executor.memory/c\spark.executor.memory '"${TERACACHE}"'g' spark.conf

sed -i '/spark.executor.cores/c\spark.executor.cores '"${CORES}" spark.conf

DRIVER_CONF="-server -XX:+UseParallelGC -XX:-UseParallelOldGC -XX:-ResizeTLAB -XX:-UseCompressedOops -XX:-UseCompressedClassPointers"
sed -i '/spark.driver.extraJavaOptions/c\spark.driver.extraJavaOptions '"${DRIVER_CONF}" spark.conf

TC_BYTES=$(echo "(${TERACACHE} - ${MIN_HEAP}) * 1024 * 1024 * 1024" | bc)
EXEC_CONF="-server -XX:-ClassUnloading -XX:+UseParallelGC -XX:-UseParallelOldGC -XX:ParallelGCThreads=16 -XX:+EnableTeraCache -XX:TeraCacheSize=${TC_BYTES} -Xms${MIN_HEAP}g -XX:TeraCacheThreshold=0 -XX:-UseCompressedOops -XX:-UseCompressedClassPointers -XX:+TeraCacheStatistics -Xlogtc:teraCache.txt -XX:TeraStripeSize=16"
sed -i '/spark.executor.extraJavaOptions/c\spark.executor.extraJavaOptions '"${EXEC_CONF}" spark.conf

# Change teracache size for Spark
if ! grep -q "teracache.heap.size" spark.conf
then
	sed -i '/spark.executor.extraJavaOptions/a\spark.teracache.heap.size '"${TERACACHE}"'g' spark.conf
else
	sed -i '/spark.teracache.heap.size/c\spark.teracache.heap.size '"${TERACACHE}"'g' spark.conf
fi

# Enable TeraCache for Spark
if ! grep -q "spark.teracache.enabled" spark.conf
then
	sed -i '/spark.teracache.heap.size/a\spark.teracache.enabled true' spark.conf
else
	sed -i '/spark.teracache.enabled/c\spark.teracache.enabled true' spark.conf
fi

# Change the spark.memory.fraction 
sed -i '/storageFraction/c\spark.memory.storageFraction '"${FRACTION}" spark.conf

cd -

if [ ${RAMDISK} -ne 0 ]
then
	cd /tmp

	# Remove the previous ramdisk
	sudo ./ramdisk_create_and_mount.sh -d
	
	# Create the new ramdisk
	MEM=$(( ${RAMDISK} * 1024 * 1024 ))
	sudo ./ramdisk_create_and_mount.sh -m ${MEM} -c

	cd -

	cd /mnt/ramdisk

	# Fill the ramdisk
	MEM=$(( ${RAMDISK} * 1024 ))
	dd if=/dev/zero of=file.txt bs=1M count=${MEM}

	cd -
fi

exit
