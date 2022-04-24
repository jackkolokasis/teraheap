#!/usr/bin/env bash                                                             

RUN_DIR=$1
CORES=$2
MEMORY=$3
STORAGE_LEVEL=$4

# KDD2012
# wget https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/binary/kdd12.bz2
# 

function timestamp()                                                            
{
    local sec=$(date +%s)
    local nanosec=$(date +%N)

    local tmp=$(expr $sec \* 1000)
    local msec=$(expr $nanosec / 1000000)
    echo $(expr $tmp + $msec)
}                                                                               

start_time=$(timestamp)

/opt/spark/spark-2.3.0-kolokasis/bin/spark-submit \
	--class org.apache.spark.examples.mllib.SparseNaiveBayes \
	--conf spark.executor.instances=1 \
	--conf spark.executor.cores=${CORES} \
	--conf spark.executor.memory=${MEMORY}g \
	--conf spark.kryoserializer.buffer.max=512m \
	--jars /opt/spark/spark-2.3.0-kolokasis/examples/target/scala-2.11/jars/spark-examples_2.11-2.3.0.jar, /opt/spark/spark-2.3.0-kolokasis/examples/target/scala-2.11/jars/scopt_2.11-3.7.0.jar\
	--numPartitions 512 \
	--numFeatures 54686452 \
    /mnt/datasets/kdd12 \
	${STORAGE_LEVEL}

end_time=$(timestamp)                                                           

duration=`echo "scale=6;($end_time-$start_time)/1000"|bc`                       

echo ",,${duration}" >> ${RUN_DIR}/total_time.txt
