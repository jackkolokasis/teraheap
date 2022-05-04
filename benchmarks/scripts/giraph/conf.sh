BENCHMARK_SUITE="/home1/public/kolokasis/graphalytics/custom_giraph_bench/graphalytics-1.2.0-giraph-0.2-SNAPSHOT"
BENCHMARK_CONFIG="${BENCHMARK_SUITE}/config"
LOG="$BENCHMARK_SUITE/report/bench.log"
HADOOP="/opt/hadoop-2.4.0"
ZOOKEEPER="/opt/zookeeper-3.4.1"
RAMDISK_SCRIPT_DIR=/tmp
RAMDISK_DIR=/mnt/ramdisk
# Directory that zookeeper use during experiment
ZOOKEEPER_DIR=/mnt/spark
# Directory that contains the file for teraheap. In case of off-heap experiments
# these directory contains the 
TH_DIR=/mnt/fmap
# Device for HDFS
DEV_HDFS=md0
# Device for Zookeeper
DEV_ZK=nvme0n1
# Device for TeraHeap or SD
DEV_TH=nvme1n1
# TeraHeap file size in GB e.g. 900 -> 900GB
TH_FILE_SZ=150
# Heap size for executors '-Xms'
HEAP=5
# Garbage collection threads
GC_THREADS=16
# Giraph number of compute threads
COMPUTE_THREADS=8
# Benchmarks to run
BENCHMARKS=( "pr" "bfs" "wcc" "cdlp" "sssp" )
#BENCHMARKS=( "pr" )
# Number of executors
EXECUTORS=1
# Number of executors
RAMDISK=0
# Total Configurations
TOTAL_CONFS=1
