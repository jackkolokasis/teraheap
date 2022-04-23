# Spark Bench Suite
# Global settings - Configurations

# Spark Master
master="sith4-fast"

# A list of machines where the spark cluster is running
MC_LIST="sith4-fast"

# Uncomment this line for sith1
#[ -z "$HADOOP_HOME" ] && export HADOOP_HOME="/opt/spark/hadoop-2.6.4"
# base dir for DataSet
#HDFS_URL="hdfs://sith0-hadoop:9000"
#SPARK_HADOOP_FS_LOCAL_BLOCK_SIZE=536870912

# DATA_HDFS="hdfs://${master}:9000/SparkBench", "file:///home/`whoami`/SparkBench"

# Use these inputs for fileio
DATA_HDFS="file:///mnt/datasets/SparkBench"

#DATA_HDFS="file:///mnt/datasets/SparkBench12"

# Use these input files for fast testing
#DATA_HDFS="file:///mnt/datasets/SparkBenchTests"

#DATA_HDFS="file:///mnt/datasets/SparkBench64"

#DATA_HDFS="file:///mnt/datasets/SparkBench128"

# Local dataset optional
DATASET_DIR="${DATA_HDFS}/dataset"

# Use this when run on Spark 2.3.0-kolokasis
SPARK_VERSION=2.3.0
[ -z "$SPARK_HOME" ] &&  export SPARK_HOME=/opt/spark/spark-2.3.0-kolokasis

SPARK_MASTER=spark://${master}:7077

SPARK_RPC_ASKTIMEOUT=10000
# Spark config in environment variable or aruments of spark-submit 
#SPARK_SERIALIZER=org.apache.spark.serializer.KryoSerializer
SPARK_RDD_COMPRESS=false
#SPARK_IO_COMPRESSION_CODEC=lzf

# Spark options in system.property or arguments of spark-submit 
SPARK_EXECUTOR_MEMORY=13g
SPARK_EXECUTOR_INSTANCES=4
SPARK_EXECUTOR_CORES=4

# Storage levels, see :
STORAGE_LEVEL=MEMORY_AND_DISK

# For data generation
NUM_OF_PARTITIONS=256

# For running
NUM_TRIALS=1
