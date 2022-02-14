# Spark Bench Suite
# Global settings - Configurations

# Spark Master
master="nextgenio-cn17"

# A list of machines where the spark cluster is running
MC_LIST="nextgenio-cn17"

# Uncomment this line for sith1
#[ -z "$HADOOP_HOME" ] && export HADOOP_HOME="/opt/spark/hadoop-2.6.4"
# base dir for DataSet
#HDFS_URL="hdfs://sith0-hadoop:9000"
#SPARK_HADOOP_FS_LOCAL_BLOCK_SIZE=536870912

# DATA_HDFS="hdfs://${master}:9000/SparkBench", "file:///home/`whoami`/SparkBench"

# 18GB Datasets 
# DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench18"

# 32GB Datasets
#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench32"

# 64GB Datasets
#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench64"

# 128GB Datasets
#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench128"

DATA_HDFS="file:///mnt/pmem_fsdax1/SparkBench"

# 256GB Datasets
#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench256"

#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench12"

#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBench6"

# Testing
#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBenchTests"

#DATA_HDFS="file:///mnt/data/SparkBench"

## This directory contains small generated datasets for testing
#DATA_HDFS="hdfs://sith0-hadoop:9000/user/kolokasis/SparkBenchTest"

# Local dataset optional
DATASET_DIR="${DATA_HDFS}/dataset"

# Use this when run on Spark 2.3.0-kolokasis
SPARK_VERSION=2.3.0
[ -z "$SPARK_HOME" ] &&  export SPARK_HOME=/home/nx05/nx05/kolokasis/TeraCacheSpark-2.3.0/spark-2.3.0-kolokasis
#[ -z "$SPARK_HOME" ] &&  export SPARK_HOME=/opt/spark/spark-panthera

# Use this when run on Spark 3.0.0-kolokasis
#SPARK_VERSION=2.3.0

#[ -z "$SPARK_HOME" ] &&  export SPARK_HOME=/opt/spark/spark-3.0.0-kolokasis

SPARK_MASTER=spark://${master}:7077

SPARK_RPC_ASKTIMEOUT=10000
# Spark config in environment variable or aruments of spark-submit 
#SPARK_SERIALIZER=org.apache.spark.serializer.KryoSerializer
SPARK_RDD_COMPRESS=false
#SPARK_IO_COMPRESSION_CODEC=lzf

# Spark options in system.property or arguments of spark-submit 
SPARK_EXECUTOR_MEMORY=900g
SPARK_EXECUTOR_INSTANCES=1
SPARK_EXECUTOR_CORES=8

# Storage levels, see :
STORAGE_LEVEL=MEMORY_ONLY

# For data generation
NUM_OF_PARTITIONS=256

# For running
NUM_TRIALS=1
